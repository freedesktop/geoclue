dbus_interface_dir = join_paths(datadir, 'dbus-1', 'interfaces')

subdir('agent')

geoclue_deps = base_deps + [ dependency('json-glib-1.0', version: '>= 0.14.0'),
                             dependency('libsoup-2.4', version: '>= 2.42.0') ]

dbus_gen_src = []

# Client interface
client_interface_xml = 'org.freedesktop.GeoClue2.Client.xml'
dbus_gen_src += gnome.gdbus_codegen(
    'gclue-client-interface',
    client_interface_xml,
    interface_prefix: 'org.freedesktop.GeoClue2.',
    namespace: 'GClueDBus',
    docbook: 'docs')
# Location interface
location_interface_xml = 'org.freedesktop.GeoClue2.Location.xml'
dbus_gen_src += gnome.gdbus_codegen(
    'gclue-location-interface',
    location_interface_xml,
    interface_prefix: 'org.freedesktop.GeoClue2.',
    namespace: 'GClueDBus',
    docbook: 'docs')
# Manager interface
manager_interface_xml = 'org.freedesktop.GeoClue2.Manager.xml'
dbus_gen_src += gnome.gdbus_codegen(
    'gclue-manager-interface',
    manager_interface_xml,
    interface_prefix: 'org.freedesktop.GeoClue2.',
    namespace: 'GClueDBus',
    docbook: 'docs')
sources = [ dbus_gen_src, libgeoclue_public_api_gen_sources[1] ]

annotations = [[ 'fi.w1.wpa_supplicant1',
                 'org.gtk.GDBus.C.Name',
                 'WPA_Supplicant' ],
               [ 'fi.w1.wpa_supplicant1.Interface',
                 'org.gtk.GDBus.C.Name',
                 'WPA_Interface' ],
               [ 'fi.w1.wpa_supplicant1.BSS',
                 'org.gtk.GDBus.C.Name',
                 'WPA_BSS' ],
               [ 'fi.w1.wpa_supplicant1.BSS:SSID',
                 'org.gtk.GDBus.C.ForceGVariant',
                 'whatever' ],
               [ 'fi.w1.wpa_supplicant1.BSS:BSSID',
                 'org.gtk.GDBus.C.ForceGVariant',
                 'whatever' ],
               [ 'fi.w1.wpa_supplicant1.Interface::BSSAdded',
                 'org.gtk.GDBus.C.Name',
                 'BSS_Added' ],
               [ 'fi.w1.wpa_supplicant1.Interface::BSSRemoved',
                 'org.gtk.GDBus.C.Name',
                 'BSS_Removed' ]]
sources += gnome.gdbus_codegen('wpa_supplicant-interface',
                               'fi.w1.wpa_supplicant1.xml',
                               interface_prefix: 'fi.w1.wpa_supplicant1.',
                               annotations: annotations)

sources += gnome.gdbus_codegen('compass-interface',
                               'net.hadess.SensorProxy.xml',
                               interface_prefix: 'net.hadess.SensorProxy')

sources += gnome.genmarshal('gclue-marshal',
                            prefix: 'gclue_marshal',
                            sources: ['gclue-marshal.list'])

include_dirs = [ configinc,
                 libgeoclue_agent_inc,
                 libgeoclue_public_api_inc,
                 include_directories('..') ]

sources += [ 'gclue-main.c',
             'gclue-3g-tower.h',
             'gclue-client-info.h', 'gclue-client-info.c',
             'gclue-compass.h', 'gclue-compass.c',
             'gclue-config.h', 'gclue-config.c',
             'gclue-error.h', 'gclue-error.c',
             'gclue-location-source.h', 'gclue-location-source.c',
             'gclue-locator.h', 'gclue-locator.c',
             'gclue-service-manager.h', 'gclue-service-manager.c',
             'gclue-service-client.h', 'gclue-service-client.c',
             'gclue-service-location.h', 'gclue-service-location.c',
             'gclue-web-source.c', 'gclue-web-source.h',
             'gclue-wifi.h', 'gclue-wifi.c',
             'gclue-mozilla.h', 'gclue-mozilla.c',
             'gclue-min-uint.h', 'gclue-min-uint.c',
             'gclue-location.h', 'gclue-location.c' ]

if get_option('3g-source') or get_option('cdma-source') or get_option('modem-gps-source')
    geoclue_deps += [ dependency('mm-glib', version: '>= 1.6') ]
    sources += [ 'gclue-modem.c',
				 'gclue-modem.h',
				 'gclue-modem-manager.c',
				 'gclue-modem-manager.h' ]
endif

if get_option('3g-source')
    sources += [ 'gclue-3g.c', 'gclue-3g.h' ]
endif

if get_option('cdma-source')
    sources += [ 'gclue-cdma.c', 'gclue-cdma.h' ]
endif

if get_option('modem-gps-source')
    sources += [ 'gclue-modem-gps.c', 'gclue-modem-gps.h' ]
endif

if get_option('nmea-source')
    geoclue_deps += [ dependency('avahi-client', version: '>= 0.6.10'),
                      dependency('avahi-glib', version: '>= 0.6.10') ]
    sources += [ 'gclue-nmea-source.h', 'gclue-nmea-source.c' ]
endif

c_args = [ '-DG_LOG_DOMAIN="Geoclue"' ]
link_with = [ libgeoclue_public_api, libgeoclue_agent ]
executable('geoclue',
           sources,
           link_with: link_with,
           include_directories: include_dirs,
           c_args: c_args,
           dependencies: geoclue_deps,
           install: true,
           install_dir: libexecdir)

dbus_interface = join_paths(dbus_interface_dir, 'org.freedesktop.GeoClue2.xml')
agent_dbus_interface = join_paths(dbus_interface_dir, 'org.freedesktop.GeoClue2.Agent.xml')
pkgconf = import('pkgconfig')
pkgconf.generate(version: gclue_version,
                 name: 'Geoclue',
                 description: 'The Geoinformation Service',
                 filebase: 'geoclue-' + gclue_api_version,
                 variables: [ 'apiversion=' + gclue_api_version,
                              'dbus_interface=' + dbus_interface,
                              'agent_dbus_interface=' + agent_dbus_interface ])

interface_files = [ location_interface_xml,
                    client_interface_xml,
                    manager_interface_xml ]
# Provide a single interface file too for backwards compatiblity.
# At least gnome-settings-daemon currently relies on that.
unified_interface_xml = 'org.freedesktop.GeoClue2.xml'
python = find_program('python3')
create_interface = find_program('create-unified-xml.py')
cmd = [python, create_interface, '@OUTPUT@', '@INPUT@' ]

custom_target('gclue-xml-interface',
              input: interface_files,
              output: unified_interface_xml,
              install: true,
              install_dir: dbus_interface_dir,
              command: cmd)
install_data(interface_files,
             install_dir: dbus_interface_dir)
