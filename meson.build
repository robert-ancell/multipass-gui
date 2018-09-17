project ('multipass-gui', 'c',
          meson_version: '>= 0.37.1')

gnome = import ('gnome')
i18n = import ('i18n')

gtk_dep = dependency ('gtk+-3.0', version: '>= 3.10')
vte_dep = dependency ('vte-2.91', version: '>= 0.48')

resources = gnome.compile_resources('mp-resources',
                                    'mp-resources.gresource.xml')
executable ('multipass-gui',
            [ 'main.c',
              'mp-client.c',
              'mp-instance-row.c',
              'mp-launch-dialog.c',
              'mp-window.c' ] + resources,
            dependencies: [ gtk_dep, vte_dep ],
            install: true)

i18n.merge_file ('desktop-file',
                 input: 'multipass-gui.desktop.in',
                 output: 'multipass-gui.desktop',
                 install: true,
                 install_dir: join_paths (get_option ('prefix'), get_option ('datadir'), 'applications'),
                 po_dir: 'po',
                 type: 'desktop')

subdir ('po')
