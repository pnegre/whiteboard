env = Environment()
env.Append(CCFLAGS = '-g')

# We should check that gcc and g++ are installed
# HOW ??

# We should check that 'pkg-config' is installed also

deps = (
	'Xtst',
	'SDL',
	'cwiid'
)

conf = Configure(env)
for l in deps:
	if not conf.CheckLib(l): exit(1)

env = conf.Finish()


libs = (
	'xtst',
	'sdl'
)

for l in libs:
	env.ParseConfig('pkg-config ' + l + ' --cflags --libs')

env.Append(LIBS = ['cwiid'])

env.Program('whiteboard', Glob('*.c*'))


