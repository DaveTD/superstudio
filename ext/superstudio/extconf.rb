require 'mkmf'

$CFLAGS << ' -std=c99'
CONFIG['warnflags'].gsub!('-Wdeclaration-after-statement', '')

create_makefile('superstudio/superstudio')