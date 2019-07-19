
Debian
====================
This directory contains files used to package peersendd/peersend-qt
for Debian-based Linux systems. If you compile peersendd/peersend-qt yourself, there are some useful files here.

## peersend: URI support ##


peersend-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install peersend-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your peersendqt binary to `/usr/bin`
and the `../../share/pixmaps/peersend128.png` to `/usr/share/pixmaps`

peersend-qt.protocol (KDE)

