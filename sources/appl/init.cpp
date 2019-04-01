/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <etk/types.hpp>
#include <ewol/ewol.hpp>
#include <ewol/object/Object.hpp>
#include <ewol/widget/Manager.hpp>

#include <appl/debug.hpp>
#include <appl/global.hpp>
#include <etk/tool.hpp>
#include <etk/path/fileSystem.hpp>
#include <etk/theme/theme.hpp>
#include <unistd.h>
//#include <ewol/config.hpp>
#include <gale/context/commandLine.hpp>
//#include <ewol/UserConfig.hpp>
#include <ewol/context/Context.hpp>
#include <appl/TextPluginManager.hpp>
#include <appl/BufferManager.hpp>
#include <appl/HighlightManager.hpp>
#include <appl/Gui/MainWindows.hpp>
#include <appl/Gui/Search.hpp>
#include <appl/ctags/readtags.hpp>
#include <appl/globalMsg.hpp>
#include <appl/TextPluginCtags.hpp>
#include <appl/debug.hpp>

class MainApplication : public ewol::context::Application {
	private:
		ememory::SharedPtr<appl::BufferManager> m_bufferManager;
		ememory::SharedPtr<appl::textPluginManager> m_pluginManager;
	public:
		virtual void onCreate(ewol::Context& _context) override {
			APPL_INFO(" == > CREATE ... " << PROJECT_NAME << "  v" << APPL_VERSION << " (START) [" << gale::getBoardType() << "] (" << gale::getCompilationMode() << ") (BEGIN)");
			for( int32_t iii=0 ; iii<_context.getCmd().size(); iii++) {
				etk::String tmpppp = _context.getCmd().get(iii);
				if (    tmpppp == "-h"
				     || tmpppp == "--help") {
					APPL_PRINT("  --ctags=xxx c-flags-file-name" );
					APPL_PRINT("  -h/--help display this help" );
					exit(0);
				} 
			}
			//etk::theme::setName("COLOR", "color/black/");
			etk::theme::setName("COLOR", "color/white/");
			
			// TODO : remove this : Move if in the windows properties
			_context.setSize(vec2(800, 600));
			
			_context.setTitle("edn");
			
			// select internal data for font ...
			_context.getFontDefault().setUseExternal(true);
			#ifdef __TARGET_OS__Android
				_context.getFontDefault().set("FreeSerif", 19);
			#else
				_context.getFontDefault().set("FreeSerif;DejaVuSansMono",14);
			#endif
			
			// set the application icon ...
			_context.setIcon("DATA:///icon.png");
			APPL_INFO("==> CREATE ... " PROJECT_NAME " (END)");
		}
		
		void onStart(ewol::Context& _context) override {
			APPL_INFO("==> START ... " PROJECT_NAME " (BEGIN)");
			// init internal global value
			globals::init();
			// init ALL Singleton :
			//()CTagsManager::getInstance();
			m_bufferManager = appl::BufferManager::create();
			m_pluginManager = appl::textPluginManager::create();
			appl::highlightManager::init();
			// Request load of the user configuration ...
			//ewol::userConfig::load();
			
			char cCurrentPath[FILENAME_MAX];
			// get the curent program folder
			if (!getcwd(cCurrentPath, FILENAME_MAX)) {
				return;
			}
			cCurrentPath[FILENAME_MAX - 1] = '\0';
			//APPL_INFO("The current working directory is " << cCurrentPath);
			
			ememory::SharedPtr<MainWindows> basicWindows = MainWindows::create();
			
			if (basicWindows == null) {
				APPL_ERROR("Can not allocate the basic windows");
				_context.exit(-1);
				return;
			}
			// create the specific windows
			_context.setWindows(basicWindows);
			
			// need to add default plugin, because they depend on the Menu widget wich might be named : "appl-menu-interface"
			m_pluginManager->addDefaultPlugin();
			
			// add files
			APPL_INFO("show list of files : ");
			for( int32_t iii=0 ; iii<_context.getCmd().size(); iii++) {
				etk::String tmpppp = _context.getCmd().get(iii);
				if (tmpppp.startWith("--ctags=") == true) {
					etk::String name = tmpppp.extract(8);
					APPL_INFO("Load ctag file : \"" << name << "\"" );
					appl::setCtagsFileName(name);
				} else if (    tmpppp == "-h"
				            || tmpppp == "--help") {
					// nothing to do ...
				} else {
					etk::Path file(tmpppp);
					if (etk::path::isFile(file) == true) {
						APPL_INFO("need load file : \"" << file << "\"" );
						m_bufferManager->open(file);
					} else if (etk::path::isDirectory(file) == true) {
						etk::Vector<etk::Path> listOfFiles = etk::path::list(file, etk::path::LIST_FILE);
						for (auto &it: listOfFiles) {
							if (etk::path::isFile(it) == true) {
								APPL_INFO("need load file : \"" << it << "\"" );
								m_bufferManager->open(it);
							}
						}
					}
				}
			}
			
			APPL_INFO("==> START ... " PROJECT_NAME " (END)");
			return;
		}
		void onStop(ewol::Context& _context) override {
			APPL_INFO("==> STOP ... " PROJECT_NAME " (START)");
			appl::highlightManager::unInit();
			//Kill all singleton
			m_pluginManager.reset();
			m_bufferManager.reset();
			APPL_INFO("==> STOP ... " PROJECT_NAME " (END)");
		}
		void onKillDemand(ewol::Context& _context) override {
			APPL_INFO("==> User demand kill ... " PROJECT_NAME " (START)");
			if (m_bufferManager == null) {
				_context.exit(0);
			}
			APPL_TODO("Implement the check of buffer not saved ...");
			_context.exit(0);
			APPL_INFO("==> User demand kill ... " PROJECT_NAME " (END)");
		}
};


/**
 * @brief Main of the program (This can be set in every case, but it is not used in Andoid...).
 * @param std IO
 * @return std IO
 */
int main(int _argc, const char *_argv[]) {
	/*
	APPL_ERROR(" base signature = " << typeid(&MainApplication::init).name());
	APPL_ERROR(" base signature = " << typeid(etk::String).name());
	APPL_CRITICAL(" END ");
	*/
	// this is really bad ...
	for( int32_t iii=0 ; iii<_argc; iii++) {
		etk::String tmpppp = _argv[iii];
		if (tmpppp.startWith("--ctags=") == true) {
			etk::String name = tmpppp.extract(8);
			APPL_INFO("Load ctag file : \"" << name << "\"" );
			appl::setCtagsFileName(name);
		}
	}
	
	
	// second possibility
	return ewol::run(ETK_NEW(MainApplication), _argc, _argv);
}


