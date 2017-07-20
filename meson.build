project('geoclue', 'c', version: '2.4.10', meson_version : '>= 0.46.1')

gclue_version = meson.project_version()
ver_arr = gclue_version.split('.')

gclue_major_version = ver_arr[0]
gclue_minor_version = ver_arr[1]
gclue_micro_version = ver_arr[2]

gclue_api_version='2.0'

conf = configuration_data()
conf.set_quoted('VERSION', gclue_version)
conf.set_quoted('PACKAGE_VERSION', gclue_version)
conf.set_quoted('PACKAGE_NAME', 'geoclue')
conf.set_quoted('GETTEXT_PACKAGE', 'geoclue')
conf.set_quoted('PACKAGE_TARNAME', 'geoclue')
conf.set_quoted('PACKAGE_STRING', 'geoclue ' + gclue_version)
conf.set_quoted('PACKAGE_URL', 'http://www.freedesktop.org/wiki/Software/GeoClue')
conf.set_quoted('PACKAGE_BUGREPORT', 'http://bugs.freedesktop.org/enter_bug.cgi?product=GeoClue')
conf.set_quoted('TEST_SRCDIR', meson.source_root() + '/data/')
datadir = get_option('prefix') + '/' + get_option('datadir')
conf.set_quoted('LOCALEDIR', datadir + '/locale')
conf.set_quoted('SYSCONFDIR', get_option('sysconfdir'))

configure_file(output: 'config.h', configuration : conf)
configinc = include_directories('.')

gnome = import('gnome')
cc = meson.get_compiler('c')

add_global_arguments([ '-Wall' ], language: 'c')

base_deps = [ dependency('glib-2.0', version: '>= 2.34.0'),
              dependency('gio-2.0', version: '>= 2.34.0'),
              dependency('gio-unix-2.0', version: '>= 2.34.0') ]
libm = cc.find_library('m', required: false)
if libm.found()
    base_deps += [ libm ]
endif

subdir('public-api')
if get_option('enable-backend')
    subdir('src')
endif
if get_option('enable-libgeoclue')
    subdir('libgeoclue')
endif
subdir('data')
subdir('demo')
subdir('po')

if get_option('enable-gtk-doc')
  subdir('docs')
endif
