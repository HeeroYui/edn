/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/debug.hpp>
#include <appl/global.hpp>
#include <appl/widget/BufferTree.hpp>
#include <appl/BufferManager.hpp>
//#include <ColorizeManager.hpp>
#include <appl/Gui/MainWindows.hpp>
#include <ewol/object/Object.hpp>
#include <ewol/compositing/Image.hpp>
#include <etk/algorithm.hpp>

appl::widget::BufferTree::BufferTree() :
  propertyShowUnNeeded(this, "show-un-needed",
                             true,
                             "show element that is not open",
                             &appl::widget::BufferTree::onChangePropertyShowUnNeeded),
  m_openOrderMode(false) {
	addObjectType("appl::BufferTree");
	m_selectedID = -1;
	m_selectedIdRequested = -1;
	// load buffer manager:
	m_bufferManager = appl::BufferManager::create();
	// load color properties
	m_paintingProperties = appl::GlyphPainting::create("THEME_COLOR:///bufferList.json");
	// get all id properties ...
	m_colorBackground1 = m_paintingProperties->request("backgroung1");
	m_colorBackground2 = m_paintingProperties->request("backgroung2");
	m_colorBackgroundHide = m_paintingProperties->request("backgroundhide");
	m_colorBackgroundSelect = m_paintingProperties->request("backgroungSelected");
	m_colorTextNormal = m_paintingProperties->request("textNormal");
	m_colorTextModify = m_paintingProperties->request("textModify");
	m_colorTextNotOpen = m_paintingProperties->request("textNotOpen");
}

void appl::widget::BufferTree::init() {
	ewol::widget::TreeView::init();
	//propertyHide.set(true);
	propertyCanFocus.set(true);
	propertyTextIsDecorated.set(false);
	if (m_bufferManager != null) {
		m_bufferManager->signalNewBuffer.connect(sharedFromThis(), &appl::widget::BufferTree::onNewBuffer);
		m_bufferManager->signalSelectFile.connect(sharedFromThis(), &appl::widget::BufferTree::onSelectBuffer);
		m_bufferManager->signalRemoveBuffer.connect(sharedFromThis(), &appl::widget::BufferTree::onRemoveBuffer);
	}
	addComposeElemnent("image_folder", ememory::makeShared<ewol::compositing::Image>("THEME_GUI:///Folder.svg?lib=ewol"));
	addComposeElemnent("image_file", ememory::makeShared<ewol::compositing::Image>("THEME_GUI:///File.svg?lib=ewol"));

}

static etk::String getCommonPathPart(const etk::Path& _left, const etk::Path& _right) {
	etk::String out;
	etk::String left = _left.getString() + "/";
	etk::String right = _right.getString() + "/";
	APPL_VERBOSE("Common Part:");
	APPL_VERBOSE("    _left  = " << left);
	APPL_VERBOSE("    _right = " << right);
	for (size_t iii=0; iii < etk::min(left.size(), right.size()); ++iii) {
		if (left[iii] == right[iii]) {
			out += left[iii];
			continue;
		}
		break;
	}
	size_t pos = out.rfind('/');
	if (pos != etk::String::npos) {
		out = out.extract(0, pos);
	}
	if (out.isEmpty() == true) {
		out = "/";
	}
	APPL_VERBOSE("    out    = " << out);
	return out;
}



void appl::widget::BufferTree::generateFlatTree() {
	// Brut Force Mode...
	etk::Path upperParent = getRootPath();
	// Now we have the root path...
	// Need to feed all elements needed.
	m_tree = etk::TreeNode<appl::TreeElement>::create(TreeElement(upperParent, true));
	populateNodeIfNeeded(m_tree);
	updateFlatTree();
}

static bool localSort(const etk::Path& _left, const etk::Path& _right) {
	return _left.getString().toUpper() <= _right.getString().toUpper();
}

void appl::widget::BufferTree::populateNodeIfNeeded(ememory::SharedPtr<etk::TreeNode<appl::TreeElement>> _node) {
	if (_node == null) {
		return;
	}
	appl::TreeElement& value = _node->getData();
	if (value.m_isFolder == false) {
		// nothing to expand...
		return;
	}
	if (_node->haveChild() == true) {
		// already populated... ==> updat list of elements: ...
		etk::Vector<etk::Path> child = etk::path::list(value.m_path, etk::path::LIST_FOLDER|etk::path::LIST_FILE);
		etk::algorithm::quickSort(child, localSort);
		APPL_VERBOSE("    nbChilds: " << child.size() << " for path: " << value.m_path);
		// Add missing element (at the end ...)
		for (auto& it: child) {
			bool find = false;
			for (auto &nodeIt: _node->getChilds()) {
				if (nodeIt->getData().m_path == it) {
					find = true;
					break;
				}
			}
			if (find == false) {
				auto elem = etk::TreeNode<appl::TreeElement>::create(TreeElement(it, false));
				_node->addChild(elem);
			}
		}
		// remove destroyed elements:
		
		size_t iii = 0;
		while (iii < _node->getChilds().size()) {
			auto node = _node->getChilds()[iii];
			bool find = false;
			for (auto& it: child) {
				if (node->getData().m_path == it) {
					find = true;
					break;
				}
			}
			if (find == false) {
				_node->rmChild(node);
			} else {
				++iii;
			}
		}
		return;
	}
	etk::Vector<etk::Path> child = etk::path::list(value.m_path, etk::path::LIST_FOLDER|etk::path::LIST_FILE);
	etk::algorithm::quickSort(child, localSort);
	APPL_VERBOSE("    nbChilds: " << child.size() << " for path: " << value.m_path);
	for (auto& it: child) {
		APPL_VERBOSE("add element: " << it);
		auto elem = etk::TreeNode<appl::TreeElement>::create(TreeElement(it, false));
		_node->addChild(elem);
	}
}

void appl::widget::BufferTree::goUpper() {
	APPL_VERBOSE("Go upper ...");
	if (m_tree == null) {
		generateFlatTree();
		return;
	}
	// generate new futur root node ...
	etk::Path path = m_tree->getData().m_path.getParent();
	if (path == m_tree->getData().m_path) {
		APPL_ERROR("No more parent in upper ... '" << path << "'");
		return;
	}
	auto treeElement = etk::TreeNode<appl::TreeElement>::create(TreeElement(path, true));
	// Add all sub-items
	populateNodeIfNeeded(treeElement);
	// find old root node in the sublist:
	for (auto& it: treeElement->getChilds()) {
		if (it->getData().m_nodeName == m_tree->getData().m_nodeName) {
			it = m_tree;
			it->setParent(treeElement);
			break;
		}
	}
	// set new root node:
	m_tree = treeElement;
	// update display:
	updateFlatTree();
}

bool appl::widget::BufferTree::updateChildOpen(ememory::SharedPtr<etk::TreeNode<appl::TreeElement>> _node) {
	if (_node == null) {
		return false;
	}
	appl::TreeElement& value = _node->getData();
	if (value.m_isFolder == false) {
		// nothing to reset...
		return value.m_buffer != null;
	}
	value.m_haveChildOpen = false;
	for (auto& it: _node->getChilds()) {
		if (updateChildOpen(it) == true) {
			value.m_haveChildOpen = true;
		}
	}
	return value.m_haveChildOpen;
}

void appl::widget::BufferTree::updateFlatTree() {
	// Enable parent with child open:
	updateChildOpen(m_tree);
	// flat tree element enable
	if (propertyShowUnNeeded.get() == false) {
		m_flatTree.setRoot(m_tree,
		    [&](const TreeElement& _value){
		    	if (    _value.m_isFolder == true
		    	     && _value.m_haveChildOpen == true) {
		    		return true;
		    	}
		    	if (    _value.m_buffer != null) {
		    		return true;
		    	}
		    	return false;
		    },
		    [&](const TreeElement& _value){
		    	return _value.m_isExpand;
		    });
	} else {
		m_flatTree.setRoot(m_tree,
		    [&](const TreeElement& _value){
		    	return true;
		    },
		    [&](const TreeElement& _value){
		    	return _value.m_isExpand;
		    });
	}
	
	markToRedraw();
}


void appl::widget::BufferTree::expandToPath(ememory::SharedPtr<etk::TreeNode<appl::TreeElement>> _node, const etk::Path& _path) {
	if (_node == null) {
		return;
	}
	appl::TreeElement& value = _node->getData();
	if (value.m_isFolder == false) {
		return;
	}
	if (_path.getString().startWith(value.m_path.getString() + "/") == false) {
		return;
	}
	// force expand
	value.m_isExpand = true;
	populateNodeIfNeeded(_node);
	for (auto& it: _node->getChilds()) {
		expandToPath(it, _path);
	}
}


appl::widget::BufferTree::~BufferTree() {
	removeAllElement();
}


void appl::widget::BufferTree::calculateMinMaxSize() {
	/*int32_t fontId = getDefaultFontId();
	int32_t minWidth = ewol::getWidth(fontId, m_label);
	int32_t minHeight = ewol::getHeight(fontId);
	m_minSize.x = 3+minWidth;
	m_minSize.y = 3+minHeight;
	*/
	m_minSize.setValue(300, 150);
}

void appl::widget::BufferTree::removeAllElement() {
	//m_list.clear();
}

etk::Path appl::widget::BufferTree::getRootPath() {
	etk::Path upperParent = "";
	etk::Vector<appl::BufferShared> tmpNewBuffer;
	for (auto& it : *m_bufferManager) {
		if (it == null) {
			continue;
		}
		if (it->hasFileName() == false) {
			tmpNewBuffer.pushBack(it);
			continue;
		}
		etk::Path nodeName = it->getFileName();
		if (upperParent.isEmpty() == true) {
			upperParent = nodeName.getParent();
			APPL_DEBUG("init root: " << nodeName << " root=" << upperParent);
			continue;
		}
		upperParent = getCommonPathPart(upperParent, nodeName.getParent());
		APPL_DEBUG("Update: " << nodeName << "   " << nodeName.getParent() << "    root=" << upperParent);
	}
	if (upperParent == "") {
		APPL_DEBUG("Nothing find ==> get home path...");
		upperParent = etk::path::getHomePath();
	}
	APPL_DEBUG("update tree: " << upperParent);
	return upperParent;
}

void appl::widget::BufferTree::onNewBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	APPL_INFO("New Buffer open: '" << _buffer->getFileName() << "'");
	ememory::SharedPtr<appl::Buffer> buffer = _buffer;
	buffer->signalIsSave.connect(sharedFromThis(), &appl::widget::BufferTree::onCallbackIsSave);
	buffer->signalIsModify.connect(sharedFromThis(), &appl::widget::BufferTree::onCallbackIsModify);
	buffer->signalChangeName.connect(sharedFromThis(), &appl::widget::BufferTree::onCallbackChangeName);
	if (m_tree == null) {
		generateFlatTree();
	} else {
		etk::Path rootPath = getRootPath();
		APPL_VERBOSE("=============================================================================================================");
		APPL_VERBOSE("== file: " << _buffer->getFileName());
		APPL_VERBOSE("== root: " << rootPath);
		APPL_VERBOSE("== tree: " << m_tree->getData().m_path);
		APPL_VERBOSE("=============================================================================================================");
		if (rootPath.getString().startWith(m_tree->getData().m_path.getString()) == false) {
			APPL_VERBOSE("    ==> not contained");
			while (    rootPath != m_tree->getData().m_path
			        && m_tree->getData().m_path.getString() != "/") {
				APPL_VERBOSE("rootPath=" << rootPath << "  !=  " << m_tree->getData().m_path);
				goUpper();
			}
		}
	}
	expandToPath(m_tree, _buffer->getFileName());
	auto listElements = m_tree->findIf([&](const etk::TreeNode<TreeElement>& _node) {
	    	APPL_VERBOSE("Compare : '" << _node.getData().m_path << "' =?= '" << _buffer->getFileName() << "'");
	    	if (_node.getData().m_path == _buffer->getFileName()) {
	    		return true;
	    	}
	    	return false;
	    });
	if (listElements.size() == 0) {
		APPL_ERROR("    ==> Can not find BUFFER ...");
	} else {
		listElements[0]->getData().m_buffer = _buffer;
	}
	updateFlatTree();
}

void appl::widget::BufferTree::onSelectBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	#if 1
		m_selection = _buffer;
		updateFlatTree();
	#else
		auto elem = m_tree.findIf([&](const TreeNode<TreeElement>& _element) {
		    	if (_element.getData().m_buffer == _buffer) {
		    		return true;
		    	}
		    	return false;
		    });
		for (auto& it: elem) {
			_element.getData().m_isSelected = true;
			// TODO: expand all this parents...
		}
		updateFlatTree();
	#endif
}

void appl::widget::BufferTree::onCallbackChangeName() {
	/*
	for (size_t iii = 0; iii < m_list.size(); ++iii) {
		if (m_list[iii].m_bufferName != m_list[iii].m_buffer->getFileName()) {
			m_list[iii].m_bufferName = m_list[iii].m_buffer->getFileName();
			if (m_openOrderMode == false) {
				// re-order the fine in the correct position
				appl::dataBufferStruct tmp = m_list[iii];
				m_list.erase(m_list.begin() + iii);
				insertAlphabetic(tmp, ((int64_t)iii == m_selectedID));
				break;
			}
		}
	}
	*/
	markToRedraw();
}

void appl::widget::BufferTree::onRemoveBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	APPL_DEBUG("request remove buffer:");
	auto elem = m_tree->findIf([&](const etk::TreeNode<TreeElement>& _element) {
	    	if (_element.getData().m_buffer == _buffer) {
	    		return true;
	    	}
	    	return false;
	    });
	for (auto& it: elem) {
		it->getData().m_isSelected = false;
		it->getData().m_buffer = null;
	}
	markToRedraw();
}

void appl::widget::BufferTree::onCallbackIsSave() {
	markToRedraw();
}

void appl::widget::BufferTree::onCallbackIsModify() {
	markToRedraw();
}

etk::Color<> appl::widget::BufferTree::getBasicBG() {
	return (*m_paintingProperties)[m_colorBackground1].getForeground();
}

ivec2 appl::widget::BufferTree::getMatrixSize() const {
	return ivec2(1, m_flatTree.size());
}

void appl::widget::BufferTree::onItemExpandEvent(const ivec2& _pos) {
	APPL_DEBUG("Event on expand on " << _pos);
	m_flatTree[_pos.y()]->getData().m_isExpand = m_flatTree[_pos.y()]->getData().m_isExpand?false:true;
	updateFlatTree();
}

fluorine::Variant appl::widget::BufferTree::getData(int32_t _role, const ivec2& _pos) {
	auto elem = m_flatTree[_pos.y()];
	appl::TreeElement& value = elem->getData();
	switch (_role) {
		case ewol::widget::ListRole::Text:
			return value.m_nodeName;
		case ewol::widget::ListRole::FgColor:
			// Folder with child open element
			if (value.m_haveChildOpen == true) {
				return (*m_paintingProperties)[m_colorTextNormal].getForeground();
			}
			if (value.m_buffer == null) {
				APPL_VERBOSE( m_colorBackgroundHide << " => " << (*m_paintingProperties)[m_colorBackgroundHide].getForeground());
				return (*m_paintingProperties)[m_colorTextNotOpen].getForeground();
			}
			if (value.m_buffer->isModify() == false) {
				return (*m_paintingProperties)[m_colorTextNormal].getForeground();
			}
			return (*m_paintingProperties)[m_colorTextModify].getForeground();
		case ewol::widget::ListRole::BgColor:
			//return fluorine::Variant();
			APPL_VERBOSE( m_colorBackground1 << " => " << (*m_paintingProperties)[m_colorBackground1].getForeground());
			if (    value.m_buffer == m_selection
			     && m_selection != null) {
				return (*m_paintingProperties)[m_colorBackgroundSelect].getForeground();
			}
			if (_pos.y() % 2) {
				return (*m_paintingProperties)[m_colorBackground1].getForeground();
			}
			return (*m_paintingProperties)[m_colorBackground2].getForeground();
		case ewol::widget::ListRole::Icon:
			/*
			if (elem->countToRoot() == 0) {
				return "THEME_GUI:///Home.svg?lib=ewol";
			}*/
			
			if (value.m_isFolder == true) {
				return "image_folder";
			} else {
				return "image_file";
			}
			return "";
		case ewol::widget::ListRole::DistanceToRoot:
			return uint_t(elem->countToRoot());
		case ewol::widget::ListRole::HaveChild:
			if (elem->haveChild() == true) {
				return true;
			}
			return value.m_haveChild;
		case ewol::widget::ListRole::IsExpand:
			return value.m_isExpand;
		case ewol::widget::ListRole::IsSelected:
			return false;
	}
	return fluorine::Variant();
}


bool appl::widget::BufferTree::onItemEvent(const ewol::event::Input& _event, const ivec2& _pos, const vec2& _mousePosition) {
	if (ewol::widget::TreeView::onItemEvent(_event, _pos, _mousePosition) == true) {
		return true;
	}
	auto elem = m_flatTree[_pos.y()];
	appl::TreeElement& value = elem->getData();
	if (_event.getId() == 1) {
		if (_event.getStatus() == gale::key::status::pressDouble) {
			if (value.m_isFolder == true) {
				populateNodeIfNeeded(m_flatTree[_pos.y()]);
				if (value.m_isExpand == true) {
					value.m_isExpand = false;
				} else {
					value.m_isExpand = true;
				}
				updateFlatTree();
				return true;
			} else if (value.m_buffer == null) {
				if (m_bufferManager != null) {
					APPL_VERBOSE("Select file: '" << value.m_path << "' in list");
					m_bufferManager->open(value.m_path);
					value.m_buffer = m_bufferManager->get(value.m_path);
				}
				return true;
			}
		}
		if (_event.getStatus() == gale::key::status::pressSingle) {
			APPL_VERBOSE("Event on List: " << _event << " pos=" << _pos );
			if (value.m_buffer != null) {
				if (m_bufferManager != null) {
					APPL_VERBOSE("Select file: '" << value.m_path << "' in list");
					m_bufferManager->open(value.m_path);
				}
				return true;
			}
		}
		/*
		if(    _pos.y() >= 0
		    && _pos.y() < (int64_t)m_list.size()) {
			if (m_list[_pos.y()].m_buffer != null) {
				if (m_bufferManager != null) {
					APPL_INFO("Select file: '" << m_list[_pos.y()].m_buffer->getFileName() << "' in list");
					m_bufferManager->open(m_list[_pos.y()].m_buffer->getFileName());
				}
				return true;
			}
		}
		*/
	}
	return false;
}

void appl::widget::BufferTree::onChangePropertyShowUnNeeded() {
	updateFlatTree();
	markToRedraw();
}
