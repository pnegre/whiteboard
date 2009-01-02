

env = Environment()
env.Append(CCFLAGS = '-g')

#deps = (
#	'cairomm-1.0',
#	'sigc-2.0',
#	'gtkmm-2.4',
#	'libglademm-2.4',
#	'Xtst',
#	'gthread-2.0'
#)
#
#conf = Configure(env)
#for l in deps:
#	#if not conf.CheckLib(l): exit(1)
#	conf.CheckLib(l)
#env = conf.Finish()


libs = (
	'xtst',
	'sdl'
)

for l in libs:
	env.ParseConfig('pkg-config ' + l + ' --cflags --libs')

env.Append(LIBS = ['cwiid'])

env.Program('whiteboard', Glob('*.c*'))


