project ('multipass-gui', 'c')

gnome = import ('gnome')

gtk_dep = dependency ('gtk+-3.0')

resources = gnome.compile_resources('mp-resources',
                                    'mp-resources.gresource.xml')
executable ('multipass-gui',
            [ 'main.c',
              'mp-instance-row.c',
              'mp-launch-dialog.c',
              'mp-window.c' ] + resources,
            dependencies: [ gtk_dep ],
            install: true)
