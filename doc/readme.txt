In order to verify git tags with 'git tag -v <tagname>' you need to have gnupg
installed and my public key imported. You can find it by searching for
"Janos Laube <janos.dev@gmail.com>" on http://pgp.mit.edu or import it directly
with gnupg via:

	gpg --keyserver pgp.mit.edu --recv-key 9f2d89fc

This key is not signed by a trusted third party yet. The fingerprint is:

	9220 d2b6 7c2d 4fc8 811a 32e7 d34e 7cb4 9f2d 89fc
