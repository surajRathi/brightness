# This is an example PKGBUILD file. Use this as a start to creating your own,
# and remove these comments. For more information, see 'man PKGBUILD'.
# NOTE: Please fill out the license field for your package! If it is unknown,
# then please put 'unknown'.

# Maintainer: Your Name <suraj.rathi00@gmail.com>
pkgname=brightness
pkgver=0.1
pkgrel=1
pkgdesc="Controls screen brightness using /sys"
arch=('any')
#url=""
license=('GPL')

source=()

build() {
	make
}


package() {
	install -Dm6511 $_pkgname "$pkgdir/usr/bin/$_pkgname"
}
