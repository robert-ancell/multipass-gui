project ('multipass-gui', 'c',
          meson_version: '>= 0.37.1')

gnome = import ('gnome')
i18n = import ('i18n')

gtk_dep = dependency ('gtk+-3.0', version: '>= 3.10')
vte_dep = dependency ('vte-2.91', version: '>= 0.48')

subdir ('po')
subdir ('src')
