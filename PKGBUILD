# Maintainer: Michael Riegert <michael at eowyn net>

pkgname='vhdlproc-git'
_pkgname='vhdlproc'
pkgver=1.1.2.0.gfc6d4e7
pkgrel=1
pkgdesc='a simple command-line VHDL preprocessor '
arch=('any')
url=https://github.com/nobodywasishere/VHDLproc
license=('GPL3')
depends=('python' 'python-argparse')
makedepends=('git')
provides=('vhdlproc')
conflicts=('vhdlproc')
source=("${pkgname}"::git+${url})
md5sums=('SKIP')

pkgver() {
    cd "${pkgname}"
    git describe --long --tags | sed 's/-/./g;s/v//g'
}

package() {
    cd "${pkgname}"
    install -Dm755 src/${_pkgname} ${pkgdir}/usr/bin/${_pkgname}
}
