# Maintainer: Marco Genualdo <marcogenualdo@live.it>
pkgname=chttp
pkgver=1.0.0
pkgrel=1
pkgdesc="A hot-reloading HTTP server with Unix socket control"
arch=('x86_64')
url="https://github.com/marcogenualdo/chttp"
license=('GPL-3.0-only')
depends=('glibc')
makedepends=('gcc' 'make' 'git')
provides=("${pkgname%-git}")
conflicts=("${pkgname%-git}")
source=("$pkgname-arch::git+https://github.com/marcogenualdo/chttp.git")
sha256sums=('SKIP')

build() {
  cd "$srcdir/$pkgname-arch"
  make
}

package() {
  cd "$srcdir/$pkgname-arch"

  # Install the binary and the shared library
  install -Dm755 chttp "$pkgdir/usr/bin/chttp"
  install -Dm755 libchttp-handlers.so "$pkgdir/usr/lib/libchttp-handlers.so"

  # Install assets
  mkdir -p "$pkgdir/usr/share/chttp-server/www"
  cp -r www/* "$pkgdir/usr/share/chttp-server/www/"

  # Fix directory permissions (Directories need +x to be "enterable")
  chmod -R u=rwX,go=rX "$pkgdir/usr/share/chttp-server/www"
}
