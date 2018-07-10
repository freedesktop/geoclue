libgeoclue_agent_sources = gnome.gdbus_codegen(
    'geoclue-agent-interface',
    'org.freedesktop.GeoClue2.Agent.xml',
    interface_prefix: 'org.freedesktop.GeoClue2.',
    namespace: 'GClue',
    docbook: 'docs')

libgeoclue_agent_inc = include_directories('.', '../..')
libgeoclue_agent = static_library('geoclue-agent',
                                  libgeoclue_agent_sources,
                                  dependencies: base_deps,
                                  include_directories: libgeoclue_agent_inc,
                                  install: false)

install_data('org.freedesktop.GeoClue2.Agent.xml',
             install_dir: dbus_interface_dir)
