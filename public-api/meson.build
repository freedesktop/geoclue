headers = [ 'gclue-enums.h' ]

header_dir = 'libgeoclue-' + gclue_api_version
install_headers(headers, subdir: header_dir)

libgeoclue_public_api_gen_sources = gnome.mkenums_simple(
    'gclue-enum-types',
    sources: headers,
    header_prefix: '#include <gclue-enums.h>',
    decorator : 'GCLUE_ENUMS_H',
    install_header: true,
    install_dir: 'include/' + header_dir)

libgeoclue_public_api_inc = include_directories('.')
libgeoclue_public_api = static_library('geoclue-public-api',
                                       libgeoclue_public_api_gen_sources,
                                       dependencies: base_deps,
                                       install: false)

libgeoclue_public_api_dep = declare_dependency(sources: libgeoclue_public_api_gen_sources,
                                               link_whole: libgeoclue_public_api)
