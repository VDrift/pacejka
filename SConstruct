env = Environment()

env.ParseConfig('pkg-config gtkmm-2.4 --cflags --libs')
env.Append(LIBPATH = ['.'])
env.Append(CCFLAGS = ['-g3'])

check_headers = ['gtkmm.h']

#conf = Configure(env)
#for header in check_headers:
#    if not conf.CheckCXXHeader(header):
#        print 'You do not have the %s headers installed. Exiting.' % header
#        Exit(1)

list = Split("""main.cpp
	configfile.cpp
	plotter.cpp
	adjustment.cpp
	mainwindow.cpp""")

env.Program('pacejka', list)
