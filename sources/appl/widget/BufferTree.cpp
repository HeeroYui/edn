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

// TODO : write it better
static void SortElementList(etk::Vector<appl::dataBufferStruct>& _list) {
	etk::Vector<appl::dataBufferStruct> tmpList = _list;
	_list.clear();
	for(size_t iii=0; iii<tmpList.size(); iii++) {
		size_t findPos = 0;
		for(size_t jjj=0; jjj<_list.size(); jjj++) {
			//EWOL_DEBUG("compare : \""<<*tmpList[iii] << "\" and \"" << *m_listDirectory[jjj] << "\"");
			if (tmpList[iii].m_bufferName.getFileName() > _list[jjj].m_bufferName.getFileName()) {
				findPos = jjj+1;
			}
		}
		//EWOL_DEBUG("position="<<findPos);
		_list.insert(_list.begin()+findPos, tmpList[iii]);
	}
}

appl::widget::BufferTree::BufferTree() :
  m_openOrderMode(false) {
	addObjectType("appl::BufferTree");
	m_selectedID = -1;
	m_selectedIdRequested = -1;
	// load buffer manager:
	m_bufferManager = appl::BufferManager::create();
	// load color properties
	m_paintingProperties = appl::GlyphPainting::create("THEME:COLOR:bufferList.json");
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
}

static etk::String getCommonPathPart(const etk::Path& _left, const etk::Path& _right) {
	etk::String out;
	for (size_t iii=0; iii < etk::min(_left.getString().size(), _right.getString().size()); ++iii) {
		if (_left.getString()[iii] == _right.getString()[iii]) {
			out += _left.getString()[iii];
			continue;
		}
		break;
	}
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
		// already populated...
		return;
	}
	etk::Vector<etk::Path> child = etk::path::list(value.m_path);
	APPL_ERROR("    nbChilds: " << child.size() << " for path: " << value.m_path);
	for (auto& it: child) {
		APPL_ERROR("add element: " << it);
		auto elem = etk::TreeNode<appl::TreeElement>::create(TreeElement(it, false));
		_node->addChild(elem);
		// TODO: ETK_FREE(etk::FSNode, it);
	}
}

void appl::widget::BufferTree::goUpper() {
	APPL_ERROR("Go upper ...");
	if (m_tree == null) {
		generateFlatTree();
		return;
	}
	// generate new futur root node ...
	etk::Path path = m_tree->getData().m_path.getParent();
	auto treeElement = etk::TreeNode<appl::TreeElement>::create(TreeElement(path, true));
	// Add all sub-items
	populateNodeIfNeeded(treeElement);
	// find old root node in the sublist:
	for (auto& it: treeElement->getChilds()) {
		if (it->getData().m_nodeName == m_tree->getData().m_nodeName) {
			it = m_tree;
			break;
		}
	}
	// set new root node:
	m_tree = treeElement;
	// update display:
	updateFlatTree();
}

void appl::widget::BufferTree::updateFlatTree() {
	m_flatTree.setRoot(m_tree,
	    [&](const TreeElement& _value){
	    	return true;
	    },
	    [&](const TreeElement& _value){
	    	return _value.m_isExpand;
	    });
	markToRedraw();
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
			APPL_ERROR("init root: " << nodeName << " root=" << upperParent);
			continue;
		}
		upperParent = getCommonPathPart(upperParent, nodeName.getParent());
		APPL_ERROR("Update: " << nodeName << "   " << nodeName.getParent() << "    root=" << upperParent);
	}
	if (upperParent == "") {
		APPL_ERROR("Nothing find ==> get home path...");
		upperParent = etk::path::getHomePath();
	}
	APPL_ERROR("update tree: " << upperParent);
	return upperParent;
}

void appl::widget::BufferTree::onNewBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	APPL_ERROR("New Buffer open: '" << _buffer->getFileName() << "'");
	if (m_tree == null) {
		generateFlatTree();
	} else {
		etk::Path rootPath = getRootPath();
		while (rootPath != m_tree->getData().m_path ) {
			goUpper();
		}
	}
	auto listElements = m_tree->findIf([&](const etk::TreeNode<TreeElement>& _node) {
	    	APPL_WARNING("Compare : '" << _node.getData().m_path << "' =?= '" << _buffer->getFileName() << "'");
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
	/*
	ememory::SharedPtr<appl::Buffer> buffer = m_bufferManager->get(_value);
	if (buffer == null) {
		APPL_ERROR("event on element nor exist : " << _value);
		return;
	}
	buffer->signalIsSave.connect(sharedFromThis(), &BufferTree::onCallbackIsSave);
	buffer->signalIsModify.connect(sharedFromThis(), &BufferTree::onCallbackIsModify);
	buffer->signalChangeName.connect(sharedFromThis(), &BufferTree::onCallbackChangeName);
	appl::dataBufferStruct tmp(_value, buffer);
	if (m_openOrderMode == true) {
		m_list.pushBack(tmp);
	} else {
		insertAlphabetic(tmp);
	}
	if (m_list.size() <= 1) {
		propertyHide.set(true);
		propertySetOnWidgetNamed("appl-Buffer-viewer-separator", "hide", "true");
	} else {
		propertyHide.set(false);
		propertySetOnWidgetNamed("appl-Buffer-viewer-separator", "hide", "false");
	}
	markToRedraw();
	*/
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
	APPL_ERROR("request remove buffer:");
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
	APPL_WARNING("Event on expand on " << _pos);
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
			if (value.m_buffer == null) {
				//APPL_ERROR( m_colorBackgroundHide << " => " << (*m_paintingProperties)[m_colorBackgroundHide].getForeground());
				return (*m_paintingProperties)[m_colorTextNotOpen].getForeground();
			}
			if (value.m_buffer->isModify() == false) {
				return (*m_paintingProperties)[m_colorTextNormal].getForeground();
			}
			return (*m_paintingProperties)[m_colorTextModify].getForeground();
		case ewol::widget::ListRole::BgColor:
			//return fluorine::Variant();
			//APPL_ERROR( m_colorBackground1 << " => " << (*m_paintingProperties)[m_colorBackground1].getForeground());
			if (_pos.y() % 2) {
				return (*m_paintingProperties)[m_colorBackground1].getForeground();
			}
			return (*m_paintingProperties)[m_colorBackground2].getForeground();
		case ewol::widget::ListRole::Icon:
			/*
			if (elem->countToRoot() == 0) {
				return "{ewol}THEME:GUI:Home.svg";
			}*/
			
			if (value.m_isFolder == true) {
				return "{ewol}THEME:GUI:Folder.svg";
			} else {
				return "{ewol}THEME:GUI:File.svg";
			}
			return "";
		case ewol::widget::ListRole::DistanceToRoot:
			return uint_t(elem->countToRoot());
		case ewol::widget::ListRole::HaveChild:
			return elem->haveChild();
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
				// TODO: Open the file...
				return true;
			}
		}
		if (_event.getStatus() == gale::key::status::pressSingle) {
			APPL_INFO("Event on List: " << _event << " pos=" << _pos );
			if (value.m_buffer != null) {
				// TODO: Display the current buffer...
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


