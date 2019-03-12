# Building Geoclue

- The following are the dependencies needed to build Geoclue2. But If Geoclue2
  is already included in your distro/OS, you should be able to use the
  package manager's command to install all build depedndancies.

  * gio (>= 2.44.0)
  * gobject-introspection
  * json-glib
  * libsoup2.4 (>= 2.42)
  * pkg-config

  Fedora:

  ```shell
  sudo dnf builddep geoclue2
  ```

  Debian and Ubuntu:

  ```shell
  sudo apt build-dep geoclue-2.0
  ```

- For a full-fledged build, you also want ModemManager (mm-glib) and
  avahi-client and avahi-glib. You want the latter two if you want to use the
  [geoclue-share app](https://wiki.gnome.org/Apps/GeoclueShare).

  Fedora:

  ```shell
  sudo dnf install ModemManager-devel
  sudo dnf install avahi-devel
  sudo dnf install avahi-glib
  ```

  Debian and Ubuntu:

  ```shell
  sudo apt install modemmanager
  sudo apt install libavahi-client3
  sudo apt install libavahi-glib1
  sudo apt install libnotify-dev
  ```

- [Install meson](https://mesonbuild.com/Getting-meson.html).

- Ensure you have a geoclue user on your system. If it already exists, you may
  need to modify ``/etc/passwd`` file to make it a login user account.

- Build and install geoclue.

  ```shell
  meson --prefix=/usr --sysconfdir /etc -Ddbus-srv-user=geoclue build
  # you may need to pass --libdir=/usr/lib64 on some systems (eg. Fedora)
  ninja -C build
  sudo ninja -C build install
  ```

- Then you can run it as:

  ```shell
  sudo su geoclue
  G_MESSAGES_DEBUG=Geoclue /usr/libexec/geoclue
  ```

  If you get the following error, make sure ``geoclue`` process is not already
  running:

  ```
  > Failed to acquire name 'org.freedesktop.GeoClue2' on system bus or lost it
  ```

- Test

  ```shell
  /usr/libexec/geoclue-2.0/demos/where-am-i
  ```

  It will give your current location.
