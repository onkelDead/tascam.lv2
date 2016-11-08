#! /usr/bin/env python
# encoding: utf-8
# WARNING! Do not edit! https://waf.io/book/index.html#_obtaining_the_waf_file

import glob
import os
import subprocess
import sys
from waflib import Build,Configure,Context,Logs,Node,Options,Task,Utils
from waflib.TaskGen import feature,before,after
global g_is_child
g_is_child=False
global g_step
g_step=0
class TestContext(Build.BuildContext):
	cmd='test'
	fun='test'
@feature('c','cxx')
@after('apply_incpaths')
def include_config_h(self):
	self.env.append_value('INCPATHS',self.bld.bldnode.abspath())
def set_options(opt,debug_by_default=False,test=False):
	global g_step
	if g_step>0:
		return
	dirs_options=opt.add_option_group('Installation directories','')
	for k in('--prefix','--destdir'):
		option=opt.parser.get_option(k)
		if option:
			opt.parser.remove_option(k)
			dirs_options.add_option(option)
	dirs_options.add_option('--bindir',type='string',help="Executable programs [Default: PREFIX/bin]")
	dirs_options.add_option('--configdir',type='string',help="Configuration data [Default: PREFIX/etc]")
	dirs_options.add_option('--datadir',type='string',help="Shared data [Default: PREFIX/share]")
	dirs_options.add_option('--includedir',type='string',help="Header files [Default: PREFIX/include]")
	dirs_options.add_option('--libdir',type='string',help="Libraries [Default: PREFIX/lib]")
	dirs_options.add_option('--mandir',type='string',help="Manual pages [Default: DATADIR/man]")
	dirs_options.add_option('--docdir',type='string',help="HTML documentation [Default: DATADIR/doc]")
	if debug_by_default:
		opt.add_option('--optimize',action='store_false',default=True,dest='debug',help="Build optimized binaries")
	else:
		opt.add_option('--debug',action='store_true',default=False,dest='debug',help="Build debuggable binaries")
		opt.add_option('--pardebug',action='store_true',default=False,dest='pardebug',help="Build parallel-installable debuggable libraries with D suffix")
	opt.add_option('--strict',action='store_true',default=False,dest='strict',help="Use strict compiler flags and show all warnings")
	opt.add_option('--ultra-strict',action='store_true',default=False,dest='ultra_strict',help="Use even stricter compiler flags (likely to trigger many warnings in library headers)")
	opt.add_option('--docs',action='store_true',default=False,dest='docs',help="Build documentation - requires doxygen")
	if test:
		test_opts=opt.add_option_group('Test options','')
		test_opts.add_option('--test',action='store_true',dest='build_tests',help='Build unit tests')
		test_opts.add_option('--no-coverage',action='store_true',dest='no_coverage',help='Do not generate lcov code coverage report')
		test_opts.add_option('--test-wrapper',type='string',dest='test_wrapper',help='Command prefix for tests (e.g. valgrind)')
		test_opts.add_option('--verbose-tests',action='store_true',default=False,dest='verbose_tests',help='Always show test output')
	g_step=1
def check_header(conf,lang,name,define='',mandatory=True):
	includes=''
	if sys.platform=="darwin":
		includes='/opt/local/include'
	if lang=='c':
		check_func=conf.check_cc
	elif lang=='cxx':
		check_func=conf.check_cxx
	else:
		Logs.error("Unknown header language `%s'"%lang)
		return
	if define!='':
		check_func(header_name=name,includes=includes,define_name=define,mandatory=mandatory)
	else:
		check_func(header_name=name,includes=includes,mandatory=mandatory)
def nameify(name):
	return name.replace('/','_').replace('++','PP').replace('-','_').replace('.','_')
def define(conf,var_name,value):
	conf.define(var_name,value)
	conf.env[var_name]=value
def check_pkg(conf,name,**args):
	if args['uselib_store'].lower()in conf.env['AUTOWAF_LOCAL_LIBS']:
		return
	class CheckType:
		OPTIONAL=1
		MANDATORY=2
	var_name='CHECKED_'+nameify(args['uselib_store'])
	check=not var_name in conf.env
	mandatory=not'mandatory'in args or args['mandatory']
	if not check and'atleast_version'in args:
		checked_version=conf.env['VERSION_'+name]
		if checked_version and checked_version<args['atleast_version']:
			check=True;
	if not check and mandatory and conf.env[var_name]==CheckType.OPTIONAL:
		check=True;
	if check:
		found=None
		pkg_var_name='PKG_'+name.replace('-','_')
		pkg_name=name
		if conf.env.PARDEBUG:
			args['mandatory']=False
			found=conf.check_cfg(package=pkg_name+'D',args="--cflags --libs",**args)
			if found:
				pkg_name+='D'
		if mandatory:
			args['mandatory']=True
		if not found:
			found=conf.check_cfg(package=pkg_name,args="--cflags --libs",**args)
		if found:
			conf.env[pkg_var_name]=pkg_name
		if'atleast_version'in args:
			conf.env['VERSION_'+name]=args['atleast_version']
	if mandatory:
		conf.env[var_name]=CheckType.MANDATORY
	else:
		conf.env[var_name]=CheckType.OPTIONAL
def normpath(path):
	if sys.platform=='win32':
		return os.path.normpath(path).replace('\\','/')
	else:
		return os.path.normpath(path)
def configure(conf):
	global g_step
	if g_step>1:
		return
	def append_cxx_flags(flags):
		conf.env.append_value('CFLAGS',flags)
		conf.env.append_value('CXXFLAGS',flags)
	print('')
	display_header('Global Configuration')
	if Options.options.docs:
		conf.load('doxygen')
	conf.env['DOCS']=Options.options.docs
	conf.env['DEBUG']=Options.options.debug or Options.options.pardebug
	conf.env['PARDEBUG']=Options.options.pardebug
	conf.env['PREFIX']=normpath(os.path.abspath(os.path.expanduser(conf.env['PREFIX'])))
	def config_dir(var,opt,default):
		if opt:
			conf.env[var]=normpath(opt)
		else:
			conf.env[var]=normpath(default)
	opts=Options.options
	prefix=conf.env['PREFIX']
	config_dir('BINDIR',opts.bindir,os.path.join(prefix,'bin'))
	config_dir('SYSCONFDIR',opts.configdir,os.path.join(prefix,'etc'))
	config_dir('DATADIR',opts.datadir,os.path.join(prefix,'share'))
	config_dir('INCLUDEDIR',opts.includedir,os.path.join(prefix,'include'))
	config_dir('LIBDIR',opts.libdir,os.path.join(prefix,'lib'))
	config_dir('MANDIR',opts.mandir,os.path.join(conf.env['DATADIR'],'man'))
	config_dir('DOCDIR',opts.docdir,os.path.join(conf.env['DATADIR'],'doc'))
	if Options.options.debug:
		if conf.env['MSVC_COMPILER']:
			conf.env['CFLAGS']=['/Od','/Zi','/MTd']
			conf.env['CXXFLAGS']=['/Od','/Zi','/MTd']
			conf.env['LINKFLAGS']=['/DEBUG']
		else:
			conf.env['CFLAGS']=['-O0','-g']
			conf.env['CXXFLAGS']=['-O0','-g']
	else:
		if conf.env['MSVC_COMPILER']:
			conf.env['CFLAGS']=['/MD']
			conf.env['CXXFLAGS']=['/MD']
		append_cxx_flags(['-DNDEBUG'])
	if Options.options.ultra_strict:
		Options.options.strict=True
		conf.env.append_value('CFLAGS',['-Wredundant-decls','-Wstrict-prototypes','-Wmissing-prototypes','-Wcast-qual'])
		conf.env.append_value('CXXFLAGS',['-Wcast-qual'])
	if Options.options.strict:
		conf.env.append_value('CFLAGS',['-pedantic','-Wshadow'])
		if conf.env.DEST_OS!="darwin":
			conf.env.append_value('LINKFLAGS',['-Wl,--no-undefined'])
		conf.env.append_value('CXXFLAGS',['-ansi','-Wnon-virtual-dtor','-Woverloaded-virtual'])
		append_cxx_flags(['-Wall','-Wcast-align','-Wextra','-Wmissing-declarations','-Wno-unused-parameter','-Wstrict-overflow','-Wundef','-Wwrite-strings','-fstrict-overflow'])
		if not conf.check_cc(fragment='''
#ifndef __clang__
#error
#endif
int main() { return 0; }''',features='c',mandatory=False,execute=False,define_name='CLANG',msg='Checking for clang'):
			append_cxx_flags(['-Wlogical-op','-Wsuggest-attribute=noreturn','-Wunsafe-loop-optimizations'])
	if not conf.env['MSVC_COMPILER']:
		append_cxx_flags(['-fshow-column'])
	conf.env.NO_COVERAGE=True
	conf.env.BUILD_TESTS=False
	try:
		conf.env.BUILD_TESTS=Options.options.build_tests
		conf.env.NO_COVERAGE=Options.options.no_coverage
		if not Options.options.no_coverage:
			if conf.is_defined('CLANG'):
				conf.find_program('llvm-cov',var='LLVM_COV',mandatory=False)
			else:
				conf.check_cc(lib='gcov',define_name='HAVE_GCOV',mandatory=False)
	except:
		pass
	conf.env.prepend_value('CFLAGS','-I'+os.path.abspath('.'))
	conf.env.prepend_value('CXXFLAGS','-I'+os.path.abspath('.'))
	display_msg(conf,"Install prefix",conf.env['PREFIX'])
	display_msg(conf,"Debuggable build",str(conf.env['DEBUG']))
	display_msg(conf,"Build documentation",str(conf.env['DOCS']))
	print('')
	g_step=2
def set_c99_mode(conf):
	if conf.env.MSVC_COMPILER:
		conf.env.append_unique('CFLAGS',['-TP'])
	else:
		conf.env.append_unique('CFLAGS',['-std=c99'])
def set_local_lib(conf,name,has_objects):
	var_name='HAVE_'+nameify(name.upper())
	define(conf,var_name,1)
	if has_objects:
		if type(conf.env['AUTOWAF_LOCAL_LIBS'])!=dict:
			conf.env['AUTOWAF_LOCAL_LIBS']={}
		conf.env['AUTOWAF_LOCAL_LIBS'][name.lower()]=True
	else:
		if type(conf.env['AUTOWAF_LOCAL_HEADERS'])!=dict:
			conf.env['AUTOWAF_LOCAL_HEADERS']={}
		conf.env['AUTOWAF_LOCAL_HEADERS'][name.lower()]=True
def append_property(obj,key,val):
	if hasattr(obj,key):
		setattr(obj,key,getattr(obj,key)+val)
	else:
		setattr(obj,key,val)
def use_lib(bld,obj,libs):
	abssrcdir=os.path.abspath('.')
	libs_list=libs.split()
	for l in libs_list:
		in_headers=l.lower()in bld.env['AUTOWAF_LOCAL_HEADERS']
		in_libs=l.lower()in bld.env['AUTOWAF_LOCAL_LIBS']
		if in_libs:
			append_property(obj,'use',' lib%s '%l.lower())
			append_property(obj,'framework',bld.env['FRAMEWORK_'+l])
		if in_headers or in_libs:
			inc_flag='-iquote '+os.path.join(abssrcdir,l.lower())
			for f in['CFLAGS','CXXFLAGS']:
				if not inc_flag in bld.env[f]:
					bld.env.prepend_value(f,inc_flag)
		else:
			append_property(obj,'uselib',' '+l)
@feature('c','cxx')
@before('apply_link')
def version_lib(self):
	if self.env.DEST_OS=='win32':
		self.vnum=None
	if self.env['PARDEBUG']:
		applicable=['cshlib','cxxshlib','cstlib','cxxstlib']
		if[x for x in applicable if x in self.features]:
			self.target=self.target+'D'
def set_lib_env(conf,name,version):
	'Set up environment for local library as if found via pkg-config.'
	NAME=name.upper()
	major_ver=version.split('.')[0]
	pkg_var_name='PKG_'+name.replace('-','_')+'_'+major_ver
	lib_name='%s-%s'%(name,major_ver)
	if conf.env.PARDEBUG:
		lib_name+='D'
	conf.env[pkg_var_name]=lib_name
	conf.env['INCLUDES_'+NAME]=['${INCLUDEDIR}/%s-%s'%(name,major_ver)]
	conf.env['LIBPATH_'+NAME]=[conf.env.LIBDIR]
	conf.env['LIB_'+NAME]=[lib_name]
def display_header(title):
	Logs.pprint('BOLD',title)
def display_msg(conf,msg,status=None,color=None):
	color='CYAN'
	if type(status)==bool and status or status=="True":
		color='GREEN'
	elif type(status)==bool and not status or status=="False":
		color='YELLOW'
	Logs.pprint('BOLD'," *",sep='')
	Logs.pprint('NORMAL',"%s"%msg.ljust(conf.line_just-3),sep='')
	Logs.pprint('BOLD',":",sep='')
	Logs.pprint(color,status)
def link_flags(env,lib):
	return' '.join(map(lambda x:env['LIB_ST']%x,env['LIB_'+lib]))
def compile_flags(env,lib):
	return' '.join(map(lambda x:env['CPPPATH_ST']%x,env['INCLUDES_'+lib]))
def set_recursive():
	global g_is_child
	g_is_child=True
def is_child():
	global g_is_child
	return g_is_child
def build_pc(bld,name,version,version_suffix,libs,subst_dict={}):
	'''Build a pkg-config file for a library.
    name           -- uppercase variable name     (e.g. 'SOMENAME')
    version        -- version string              (e.g. '1.2.3')
    version_suffix -- name version suffix         (e.g. '2')
    libs           -- string/list of dependencies (e.g. 'LIBFOO GLIB')
    '''
	pkg_prefix=bld.env['PREFIX']
	if pkg_prefix[-1]=='/':
		pkg_prefix=pkg_prefix[:-1]
	target=name.lower()
	if version_suffix!='':
		target+='-'+version_suffix
	if bld.env['PARDEBUG']:
		target+='D'
	target+='.pc'
	libdir=bld.env['LIBDIR']
	if libdir.startswith(pkg_prefix):
		libdir=libdir.replace(pkg_prefix,'${exec_prefix}')
	includedir=bld.env['INCLUDEDIR']
	if includedir.startswith(pkg_prefix):
		includedir=includedir.replace(pkg_prefix,'${prefix}')
	obj=bld(features='subst',source='%s.pc.in'%name.lower(),target=target,install_path=os.path.join(bld.env['LIBDIR'],'pkgconfig'),exec_prefix='${prefix}',PREFIX=pkg_prefix,EXEC_PREFIX='${prefix}',LIBDIR=libdir,INCLUDEDIR=includedir)
	if type(libs)!=list:
		libs=libs.split()
	subst_dict[name+'_VERSION']=version
	subst_dict[name+'_MAJOR_VERSION']=version[0:version.find('.')]
	for i in libs:
		subst_dict[i+'_LIBS']=link_flags(bld.env,i)
		lib_cflags=compile_flags(bld.env,i)
		if lib_cflags=='':
			lib_cflags=' '
		subst_dict[i+'_CFLAGS']=lib_cflags
	obj.__dict__.update(subst_dict)
def build_dir(name,subdir):
	if is_child():
		return os.path.join('build',name,subdir)
	else:
		return os.path.join('build',subdir)
def make_simple_dox(name):
	name=name.lower()
	NAME=name.upper()
	try:
		top=os.getcwd()
		os.chdir(build_dir(name,'doc/html'))
		page='group__%s.html'%name
		if not os.path.exists(page):
			return
		for i in[['%s_API '%NAME,''],['%s_DEPRECATED '%NAME,''],['group__%s.html'%name,''],['&#160;',''],['<script.*><\/script>',''],['<hr\/><a name="details" id="details"><\/a><h2>.*<\/h2>',''],['<link href=\"tabs.css\" rel=\"stylesheet\" type=\"text\/css\"\/>',''],['<img class=\"footer\" src=\"doxygen.png\" alt=\"doxygen\"\/>','Doxygen']]:
			os.system("sed -i 's/%s/%s/g' %s"%(i[0],i[1],page))
		os.rename('group__%s.html'%name,'index.html')
		for i in(glob.glob('*.png')+glob.glob('*.html')+glob.glob('*.js')+glob.glob('*.css')):
			if i!='index.html'and i!='style.css':
				os.remove(i)
		os.chdir(top)
		os.chdir(build_dir(name,'doc/man/man3'))
		for i in glob.glob('*.3'):
			os.system("sed -i 's/%s_API //' %s"%(NAME,i))
		for i in glob.glob('_*'):
			os.remove(i)
		os.chdir(top)
	except Exception ,e:
		Logs.error("Failed to fix up %s documentation: %s"%(name,e))
def build_dox(bld,name,version,srcdir,blddir,outdir='',versioned=True):
	if not bld.env['DOCS']:
		return
	if is_child():
		src_dir=os.path.join(srcdir,name.lower())
	else:
		src_dir=srcdir
	subst_tg=bld(features='subst',source='doc/reference.doxygen.in',target='doc/reference.doxygen',install_path='',name='doxyfile')
	subst_dict={name+'_VERSION':version,name+'_SRCDIR':os.path.abspath(src_dir),name+'_DOC_DIR':''}
	subst_tg.__dict__.update(subst_dict)
	subst_tg.post()
	docs=bld(features='doxygen',doxyfile='doc/reference.doxygen')
	docs.post()
	outname=name.lower()
	if versioned:
		outname+='-%d'%int(version[0:version.find('.')])
	bld.install_files(os.path.join('${DOCDIR}',outname,outdir,'html'),bld.path.get_bld().ant_glob('doc/html/*'))
	for i in range(1,8):
		bld.install_files('${MANDIR}/man%d'%i,bld.path.get_bld().ant_glob('doc/man/man%d/*'%i,excl='**/_*'))
def build_version_files(header_path,source_path,domain,major,minor,micro):
	header_path=os.path.abspath(header_path)
	source_path=os.path.abspath(source_path)
	text="int "+domain+"_major_version = "+str(major)+";\n"
	text+="int "+domain+"_minor_version = "+str(minor)+";\n"
	text+="int "+domain+"_micro_version = "+str(micro)+";\n"
	try:
		o=open(source_path,'w')
		o.write(text)
		o.close()
	except IOError:
		Logs.error('Failed to open %s for writing\n'%source_path)
		sys.exit(-1)
	text="#ifndef __"+domain+"_version_h__\n"
	text+="#define __"+domain+"_version_h__\n"
	text+="extern const char* "+domain+"_revision;\n"
	text+="extern int "+domain+"_major_version;\n"
	text+="extern int "+domain+"_minor_version;\n"
	text+="extern int "+domain+"_micro_version;\n"
	text+="#endif /* __"+domain+"_version_h__ */\n"
	try:
		o=open(header_path,'w')
		o.write(text)
		o.close()
	except IOError:
		Logs.warn('Failed to open %s for writing\n'%header_path)
		sys.exit(-1)
	return None
def build_i18n_pot(bld,srcdir,dir,name,sources,copyright_holder=None):
	Logs.info('Generating pot file from %s'%name)
	pot_file='%s.pot'%name
	cmd=['xgettext','--keyword=_','--keyword=N_','--keyword=S_','--from-code=UTF-8','-o',pot_file]
	if copyright_holder:
		cmd+=['--copyright-holder="%s"'%copyright_holder]
	cmd+=sources
	Logs.info('Updating '+pot_file)
	subprocess.call(cmd,cwd=os.path.join(srcdir,dir))
def build_i18n_po(bld,srcdir,dir,name,sources,copyright_holder=None):
	pwd=os.getcwd()
	os.chdir(os.path.join(srcdir,dir))
	pot_file='%s.pot'%name
	po_files=glob.glob('po/*.po')
	for po_file in po_files:
		cmd=['msgmerge','--update',po_file,pot_file]
		Logs.info('Updating '+po_file)
		subprocess.call(cmd)
	os.chdir(pwd)
def build_i18n_mo(bld,srcdir,dir,name,sources,copyright_holder=None):
	pwd=os.getcwd()
	os.chdir(os.path.join(srcdir,dir))
	pot_file='%s.pot'%name
	po_files=glob.glob('po/*.po')
	for po_file in po_files:
		mo_file=po_file.replace('.po','.mo')
		cmd=['msgfmt','-c','-f','-o',mo_file,po_file]
		Logs.info('Generating '+po_file)
		subprocess.call(cmd)
	os.chdir(pwd)
def build_i18n(bld,srcdir,dir,name,sources,copyright_holder=None):
	build_i18n_pot(bld,srcdir,dir,name,sources,copyright_holder)
	build_i18n_po(bld,srcdir,dir,name,sources,copyright_holder)
	build_i18n_mo(bld,srcdir,dir,name,sources,copyright_holder)
def cd_to_build_dir(ctx,appname):
	orig_dir=os.path.abspath(os.curdir)
	top_level=(len(ctx.stack_path)>1)
	if top_level:
		os.chdir(os.path.join('build',appname))
	else:
		os.chdir('build')
	Logs.pprint('GREEN',"Waf: Entering directory `%s'"%os.path.abspath(os.getcwd()))
def cd_to_orig_dir(ctx,child):
	if child:
		os.chdir(os.path.join('..','..'))
	else:
		os.chdir('..')
def pre_test(ctx,appname,dirs=['src']):
	if not hasattr(ctx,'autowaf_tests_total'):
		ctx.autowaf_tests_total=0
		ctx.autowaf_tests_failed=0
		ctx.autowaf_local_tests_total=0
		ctx.autowaf_local_tests_failed=0
		ctx.autowaf_tests={}
	ctx.autowaf_tests[appname]={'total':0,'failed':0}
	cd_to_build_dir(ctx,appname)
	if not ctx.env.NO_COVERAGE:
		diropts=''
		for i in dirs:
			diropts+=' -d '+i
		clear_log=open('lcov-clear.log','w')
		try:
			try:
				subprocess.call(('lcov %s -z'%diropts).split(),stdout=clear_log,stderr=clear_log)
			except:
				Logs.warn('Failed to run lcov, no coverage report will be generated')
		finally:
			clear_log.close()
def post_test(ctx,appname,dirs=['src'],remove=['*boost*','c++*']):
	if not ctx.env.NO_COVERAGE:
		diropts=''
		for i in dirs:
			diropts+=' -d '+i
		coverage_log=open('lcov-coverage.log','w')
		coverage_lcov=open('coverage.lcov','w')
		coverage_stripped_lcov=open('coverage-stripped.lcov','w')
		try:
			try:
				base='.'
				if g_is_child:
					base='..'
				lcov_cmd='lcov -c %s -b %s'%(diropts,base)
				if ctx.env.LLVM_COV:
					lcov_cmd+=' --gcov-tool %s'%ctx.env.LLVM_COV[0]
				subprocess.call(lcov_cmd.split(),stdout=coverage_lcov,stderr=coverage_log)
				subprocess.call(['lcov','--remove','coverage.lcov']+remove,stdout=coverage_stripped_lcov,stderr=coverage_log)
				if not os.path.isdir('coverage'):
					os.makedirs('coverage')
				subprocess.call('genhtml -o coverage coverage-stripped.lcov'.split(),stdout=coverage_log,stderr=coverage_log)
			except:
				Logs.warn('Failed to run lcov, no coverage report will be generated')
		finally:
			coverage_stripped_lcov.close()
			coverage_lcov.close()
			coverage_log.close()
	if ctx.autowaf_tests[appname]['failed']>0:
		Logs.pprint('RED','\nSummary:  %d / %d %s tests failed'%(ctx.autowaf_tests[appname]['failed'],ctx.autowaf_tests[appname]['total'],appname))
	else:
		Logs.pprint('GREEN','\nSummary:  All %d %s tests passed'%(ctx.autowaf_tests[appname]['total'],appname))
	if not ctx.env.NO_COVERAGE:
		Logs.pprint('GREEN','Coverage: <file://%s>\n'%os.path.abspath('coverage/index.html'))
	Logs.pprint('GREEN',"Waf: Leaving directory `%s'"%os.path.abspath(os.getcwd()))
	top_level=(len(ctx.stack_path)>1)
	if top_level:
		cd_to_orig_dir(ctx,top_level)
def run_test(ctx,appname,test,desired_status=0,dirs=['src'],name='',header=False):
	ctx.autowaf_tests_total+=1
	ctx.autowaf_local_tests_total+=1
	ctx.autowaf_tests[appname]['total']+=1
	if type(test)==list:
		name=test[0]
		returncode=test[1]
	else:
		s=test
		if type(test)==type([]):
			s=' '.join(i)
		if header:
			Logs.pprint('Green','\n** Test %s'%s)
		cmd=test
		if Options.options.test_wrapper:
			cmd=Options.options.test_wrapper+' '+test
		if name=='':
			name=test
		if Options.options.verbose_tests:
			proc=subprocess.Popen(cmd,shell=True)
		else:
			proc=subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
		out=proc.communicate()
		returncode=proc.returncode
	if returncode==desired_status:
		Logs.pprint('GREEN','** Pass %s'%name)
		return True
	else:
		Logs.pprint('RED','** FAIL %s'%name)
		if type(test)!=list:
			Logs.pprint('NORMAL',out[1])
		ctx.autowaf_tests_failed+=1
		ctx.autowaf_tests[appname]['failed']+=1
		return False
def tests_name(ctx,appname,name='*'):
	if name=='*':
		return appname
	else:
		return'%s.%s'%(appname,name)
def begin_tests(ctx,appname,name='*'):
	ctx.autowaf_local_tests_failed=0
	ctx.autowaf_local_tests_total=0
	Logs.pprint('GREEN','\n** Begin %s tests'%tests_name(ctx,appname,name))
def end_tests(ctx,appname,name='*'):
	failures=ctx.autowaf_local_tests_failed
	if failures==0:
		Logs.pprint('GREEN','** Passed all %d %s tests'%(ctx.autowaf_local_tests_total,tests_name(ctx,appname,name)))
	else:
		Logs.pprint('RED','** Failed %d / %d %s tests'%(failures,ctx.autowaf_local_tests_total,tests_name(ctx,appname,name)))
def run_tests(ctx,appname,tests,desired_status=0,dirs=['src'],name='*',headers=False):
	begin_tests(ctx,appname,name)
	diropts=''
	for i in dirs:
		diropts+=' -d '+i
	for i in tests:
		run_test(ctx,appname,i,desired_status,dirs,i,headers)
	end_tests(ctx,appname,name)
def run_ldconfig(ctx):
	if(ctx.cmd=='install'and not ctx.env['RAN_LDCONFIG']and ctx.env['LIBDIR']and not'DESTDIR'in os.environ and not Options.options.destdir):
		try:
			Logs.info("Waf: Running `/sbin/ldconfig %s'"%ctx.env['LIBDIR'])
			subprocess.call(['/sbin/ldconfig',ctx.env['LIBDIR']])
			ctx.env['RAN_LDCONFIG']=True
		except:
			pass
def get_rdf_news(name,in_files,top_entries=None,extra_entries=None,dev_dist=None):
	import rdflib
	doap=rdflib.Namespace('http://usefulinc.com/ns/doap#')
	dcs=rdflib.Namespace('http://ontologi.es/doap-changeset#')
	rdfs=rdflib.Namespace('http://www.w3.org/2000/01/rdf-schema#')
	foaf=rdflib.Namespace('http://xmlns.com/foaf/0.1/')
	rdf=rdflib.Namespace('http://www.w3.org/1999/02/22-rdf-syntax-ns#')
	m=rdflib.ConjunctiveGraph()
	try:
		for i in in_files:
			m.parse(i,format='n3')
	except:
		Logs.warn('Error parsing data, unable to generate NEWS')
		return
	proj=m.value(None,rdf.type,doap.Project)
	for f in m.triples([proj,rdfs.seeAlso,None]):
		if f[2].endswith('.ttl'):
			m.parse(f[2],format='n3')
	entries={}
	for r in m.triples([proj,doap.release,None]):
		release=r[2]
		revision=m.value(release,doap.revision,None)
		date=m.value(release,doap.created,None)
		blamee=m.value(release,dcs.blame,None)
		changeset=m.value(release,dcs.changeset,None)
		dist=m.value(release,doap['file-release'],None)
		if not dist:
			Logs.warn('No file release for %s %s'%(proj,revision))
			dist=dev_dist
		if revision and date and blamee and changeset:
			entry={}
			entry['name']=str(name)
			entry['revision']=str(revision)
			entry['date']=str(date)
			entry['status']='stable'if dist!=dev_dist else'unstable'
			entry['dist']=str(dist)
			entry['items']=[]
			for i in m.triples([changeset,dcs.item,None]):
				item=str(m.value(i[2],rdfs.label,None))
				entry['items']+=[item]
				if dist and top_entries is not None:
					if not str(dist)in top_entries:
						top_entries[str(dist)]={'items':[]}
					top_entries[str(dist)]['items']+=['%s: %s'%(name,item)]
			if extra_entries and dist:
				for i in extra_entries[str(dist)]:
					entry['items']+=extra_entries[str(dist)]['items']
			entry['blamee_name']=str(m.value(blamee,foaf.name,None))
			entry['blamee_mbox']=str(m.value(blamee,foaf.mbox,None))
			entries[(str(date),str(revision))]=entry
		else:
			Logs.warn('Ignored incomplete %s release description'%name)
	return entries
def write_news(entries,out_file):
	import textwrap
	from time import strftime,strptime
	if len(entries)==0:
		return
	news=open(out_file,'w')
	for e in sorted(entries.keys(),reverse=True):
		entry=entries[e]
		news.write('%s (%s) %s;\n'%(entry['name'],entry['revision'],entry['status']))
		for item in entry['items']:
			wrapped=textwrap.wrap(item,width=79)
			news.write('\n  * '+'\n    '.join(wrapped))
		news.write('\n\n --')
		news.write(' %s <%s>'%(entry['blamee_name'],entry['blamee_mbox'].replace('mailto:','')))
		news.write('  %s\n\n'%(strftime('%a, %d %b %Y %H:%M:%S +0000',strptime(entry['date'],'%Y-%m-%d'))))
	news.close()
def write_posts(entries,meta,out_dir):
	for i in entries:
		entry=entries[i]
		date=i[0]
		revision=i[1]
		path=os.path.join(out_dir,'%s-%s-%s.md'%(date,entry['name'],revision))
		post=open(path,'w')
		post.write('Title: %s %s\n'%(entry['name'],revision))
		post.write('Date: %s\n'%(date))
		post.write('Slug: %s-%s\n'%(entry['name'],revision.replace('.','-')))
		for k in meta:
			post.write('%s: %s\n'%(k,meta[k]))
		post.write('\n')
		url=entry['dist']
		if entry['status']=='stable':
			post.write('[%s %s](%s) has been released.\n\n'%((entry['name'],revision,url)))
		else:
			post.write('%s %s is the current development version.\n\n'%(entry['name'],revision))
		post.write('Changes:\n\n')
		for i in entry['items']:
			post.write(' * %s\n'%i)
		post.close()
