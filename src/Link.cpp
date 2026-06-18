#include "Link.h"

#ifdef ALL_MENS_MORRIS_FXC_LINK
#define I2C_IMPLEMENTATION
#define I2C_BUFFER_SIZE 12
#include <ArduboyI2C.h>

namespace {
constexpr uint8_t LINK_MAGIC = 0xA9;
constexpr uint8_t LINK_VERSION = 1;
constexpr uint8_t LINK_KIND_BEACON = 1;
constexpr uint8_t LINK_KIND_START = 2;
constexpr uint8_t LINK_KIND_ACTION = 3;
constexpr uint8_t LINK_ADDRESS = 0x08;
constexpr uint8_t LINK_PEER_TIMEOUT_FRAMES = 45;
constexpr uint8_t LINK_SEND_INTERVAL_FRAMES = 8;
constexpr uint8_t LINK_SEND_JITTER_FRAMES = 7;
constexpr uint8_t LINK_TX_STUCK_FRAMES = 3;

struct LinkPacket {
  uint8_t magic;
  uint8_t version;
  uint8_t kind;
  uint8_t seq;
  uint8_t nonce;
  uint8_t board;
  uint8_t firstPlayer;
  uint8_t mode;
  uint8_t from;
  uint8_t to;
};

volatile LinkPacket receivedPacket = {};
volatile bool hasReceivedPacket = false;

uint8_t localNonce = 1;
uint8_t sendFrame = 0;
uint8_t sendInterval = LINK_SEND_INTERVAL_FRAMES;
uint8_t sequence = 0;
uint8_t peerTimeout = 0;
uint8_t peerNonce = 0;
uint8_t lastEventSeq = 255;
bool peerAvailable = false;
bool pendingSendStart = false;
bool pendingSendAction = false;
bool linkStarted = false;
uint8_t txActiveFrames = 0;
LinkPacket pendingAction = {};
LinkEvent pendingEvent = {};

void onReceive();

void linkResetBus() {
  i2c_detail::data.active = false;
  I2C::init();
  I2C::setAddress(LINK_ADDRESS, false);
  I2C::onReceive(onReceive);
  txActiveFrames = 0;
}

bool linkBusReady() {
  if (!linkStarted) {
    return false;
  }
  if (!i2c_detail::data.active) {
    txActiveFrames = 0;
    return true;
  }
  if (++txActiveFrames >= LINK_TX_STUCK_FRAMES) {
    linkResetBus();
  }
  return false;
}

void onReceive() {
  const uint8_t *buffer = I2C::getBuffer();
  const LinkPacket *packet = reinterpret_cast<const LinkPacket *>(buffer);
  if (packet->magic != LINK_MAGIC || packet->version != LINK_VERSION) {
    return;
  }
  receivedPacket = *packet;
  hasReceivedPacket = true;
}

void sendPacket(LinkPacket packet) {
  if (!linkBusReady()) {
    return;
  }
  packet.magic = LINK_MAGIC;
  packet.version = LINK_VERSION;
  packet.seq = sequence++;
  packet.nonce = localNonce;
  I2C::write(LINK_ADDRESS, &packet, false);
  if (I2C::getTWError() != TW_SUCCESS) {
    i2c_detail::data.active = false;
  }
}

uint8_t nextSendInterval() {
  return LINK_SEND_INTERVAL_FRAMES + random(LINK_SEND_JITTER_FRAMES + 1);
}

void queueEvent(const LinkPacket &packet) {
  if (packet.seq == lastEventSeq) {
    return;
  }
  lastEventSeq = packet.seq;
  pendingEvent.board = packet.board;
  pendingEvent.firstPlayer = static_cast<Player>(packet.firstPlayer);
  pendingEvent.mode = static_cast<TurnActionMode>(packet.mode);
  pendingEvent.from = packet.from;
  pendingEvent.to = packet.to;
  if (packet.kind == LINK_KIND_START) {
    pendingEvent.kind = LINK_EVENT_START;
  } else if (packet.kind == LINK_KIND_ACTION) {
    pendingEvent.kind = LINK_EVENT_ACTION;
  }
}

void processReceived() {
  if (!hasReceivedPacket) {
    return;
  }
  noInterrupts();
  LinkPacket packet = {};
  packet.magic = receivedPacket.magic;
  packet.version = receivedPacket.version;
  packet.kind = receivedPacket.kind;
  packet.seq = receivedPacket.seq;
  packet.nonce = receivedPacket.nonce;
  packet.board = receivedPacket.board;
  packet.firstPlayer = receivedPacket.firstPlayer;
  packet.mode = receivedPacket.mode;
  packet.from = receivedPacket.from;
  packet.to = receivedPacket.to;
  hasReceivedPacket = false;
  interrupts();

  if (packet.nonce == localNonce) {
    return;
  }
  peerAvailable = true;
  peerTimeout = LINK_PEER_TIMEOUT_FRAMES;
  peerNonce = packet.nonce;
  if (packet.kind == LINK_KIND_START || packet.kind == LINK_KIND_ACTION) {
    queueEvent(packet);
  }
}

void sendBeacon(uint8_t board, Player firstPlayer) {
  LinkPacket packet = {};
  packet.kind = LINK_KIND_BEACON;
  packet.board = board;
  packet.firstPlayer = firstPlayer;
  packet.mode = TURN_ACTION_MOVE;
  packet.from = 255;
  packet.to = 255;
  sendPacket(packet);
}
}

void linkBegin(uint32_t seed) {
  localNonce = static_cast<uint8_t>((seed ^ (seed >> 8) ^ (seed >> 16)) & 0x7F);
  if (localNonce == 0) {
    localNonce = 1;
  }
  sendFrame = localNonce % LINK_SEND_INTERVAL_FRAMES;
  sendInterval = nextSendInterval();
  linkStarted = true;
  linkResetBus();
}

void linkUpdate(bool inMainMenu, uint8_t board, Player firstPlayer) {
  linkBusReady();
  processReceived();
  if (peerTimeout > 0) {
    peerTimeout--;
  } else {
    peerAvailable = false;
  }

  if (pendingSendStart) {
    pendingSendStart = false;
    LinkPacket packet = {};
    packet.kind = LINK_KIND_START;
    packet.board = board;
    packet.firstPlayer = firstPlayer;
    packet.mode = TURN_ACTION_MOVE;
    packet.from = 255;
    packet.to = 255;
    sendPacket(packet);
    return;
  }
  if (pendingSendAction) {
    pendingSendAction = false;
    sendPacket(pendingAction);
    return;
  }
  if (inMainMenu && sendFrame++ >= sendInterval) {
    sendFrame = 0;
    sendInterval = nextSendInterval();
    sendBeacon(board, firstPlayer);
  }
}

bool linkPeerAvailable() {
  return peerAvailable;
}

bool linkLocalIsPlayerOne() {
  return localNonce < peerNonce;
}

Player linkLocalPlayer(Player firstPlayer) {
  return linkLocalIsPlayerOne() ? firstPlayer : opponentOf(firstPlayer);
}

bool linkConsumeEvent(LinkEvent &event) {
  if (pendingEvent.kind == LINK_EVENT_NONE) {
    return false;
  }
  event = pendingEvent;
  pendingEvent.kind = LINK_EVENT_NONE;
  return true;
}

void linkSendStart(uint8_t, Player) {
  pendingSendStart = true;
}

void linkSendAction(TurnActionMode mode, uint8_t from, uint8_t to) {
  pendingAction = {};
  pendingAction.kind = LINK_KIND_ACTION;
  pendingAction.mode = mode;
  pendingAction.from = from;
  pendingAction.to = to;
  pendingSendAction = true;
}

#else

void linkBegin(uint32_t) {}
void linkUpdate(bool, uint8_t, Player) {}
bool linkPeerAvailable() { return false; }
bool linkLocalIsPlayerOne() { return true; }
Player linkLocalPlayer(Player firstPlayer) { return firstPlayer; }
bool linkConsumeEvent(LinkEvent &) { return false; }
void linkSendStart(uint8_t, Player) {}
void linkSendAction(TurnActionMode, uint8_t, uint8_t) {}

#endif
