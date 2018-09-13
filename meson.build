project ('multipass-gui', 'c')

gtk_dep = dependency ('gtk+-3.0')

executable ('multipass-gui',
            [ 'main.c',
              'mp-window.c' ],
            dependencies: [ gtk_dep ],
            install: true)
