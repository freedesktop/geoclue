subdir('lib')

conf = configuration_data()
conf.set_quoted('VERSION', gclue_version)
version_file = configure_file(output: 'version.xml',
                              input: 'version.xml.in',
                              configuration: conf)

content_files = [ '../COPYING',
                  dbus_gen_src ]
scan_options = [ '--ignore-headers=config.h',
                 '--rebuild-sections',
                 '--rebuild-types' ]
# Extra options to supply to gtkdoc-mkdb
mkdb_options = [ '--output-format=xml' ]

gnome.gtkdoc('geoclue',
             main_xml: 'geoclue-docs.xml',
             content_files: content_files,
             scan_args: scan_options,
             src_dir: 'public-api',
             mkdb_args: mkdb_options,
             install: true)
