sources = [ 'geocode-location.h',
            'geocode-location.c',
            'geocode-forward.h',
            'geocode-forward.c',
            'geocode-reverse.h',
            'geocode-reverse.c',
            'geocode-glib.h',
            'geocode-glib.c',
            'geocode-error.h',
            'geocode-error.c',
            'geocode-enum-types.h',
            'geocode-enum-types.c',
            'geocode-place.h',
            'geocode-place.c',
            'geocode-bounding-box.h',
            'geocode-bounding-box.c',
            'geocode-glib-private.h' ]

datadir = get_option('prefix') + '/' + get_option('datadir')
localedir = datadir + '/locale'

c_args = [ '-DPACKAGE_VERSION="' + gclue_version + '"',
           '-DGEOCODE_LOCALEDIR="' + localedir + '"' ]
libgeocode_glib_inc = include_directories('.', '..', '../..')
libgeocode_glib = static_library('geocode-glib',
                                 sources,
                                 dependencies: geoclue_deps,
                                 include_directories: libgeocode_glib_inc,
                                 c_args: c_args,
                                 install: false)
