project(
    'screenshotter',
    'c',
    version : '0.1.0',
    default_options : [
      'warning_level=3',
    ],
)

executable(
    'screenshotter',
    'main.c',
    install : true,
    c_args: ['-DUNICODE=1'],
    link_args: ['-lgdi32', '-luser32']
)
