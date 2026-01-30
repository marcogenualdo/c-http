# Maintainer: Marco Genualdo <marcogenualdo@live.it>
pkgname=c-http
pkgver=1.0.0
pkgrel=1
pkgdesc="A hot-reloading HTTP server with Unix socket control"
arch=('x86_64')
url="https://github.com/marcogenualdo/c-http"
license=('GPL-3.0-only')
depends=('glibc')
makedepends=('gcc' 'make')
source=("$pkgname-$pkgver.tar.gz")

build() {
  cd "$srcdir"
  make
}

package() {
  cd "$srcdir"

  # Install the binary and the shared library
  install -Dm755 server "$pkgdir/usr/bin/c-http"
  install -Dm755 libchttp-handlers.so "$pkgdir/usr/lib/libchttp-handlers.so"

  # Install assets
  mkdir -p "$pkgdir/usr/share/chttp-server/www"
  cp -r www/* "$pkgdir/usr/share/chttp-server/www/"

  # Fix directory permissions (Directories need +x to be "enterable")
  chmod -R u=rwX,go=rX "$pkgdir/usr/share/chttp-server/www"
}
sha256sums=('73448845af219e2e914e3e8ca0f32c77af82bbebb95933acced5a450dea00f4a')
