# preprocess the include/teng/config.h file, but the placing matters
conf_data = configuration_data()
version = meson.project_version().split('.')

teng_major = version[0]
teng_minor = version[1]

conf_data.set('TENG_MAJOR', teng_major)
conf_data.set('TENG_MINOR', teng_minor)

config_h = configure_file(input : 'config.h.in',
                          output : 'config.h',
                          configuration : conf_data)
