# Menu Music Source

`sources/entertainer.mid` and `sources/entertainer.ly` come from the Mutopia
Project edition of Scott Joplin's "The Entertainer".

- Source page: https://www.mutopiaproject.org/cgibin/piece-info.cgi?id=263
- Mutopia source note: reproduction of the original 1902 edition.
- Copyright status on Mutopia: public domain.

Run `make music-data` to regenerate `src/MenuMusic.*` from the MIDI source.

After TableTop Studio edits the menu music, `tools/music/menu-music.json`
becomes the editable source used by `make music-data`. Use
`tools/music/generate_menu_music.py --force-midi` to rebuild from the Mutopia
MIDI source again.
