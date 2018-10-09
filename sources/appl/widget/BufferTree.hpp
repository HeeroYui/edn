/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once

#include <appl/debug.hpp>
#include <appl/BufferManager.hpp>
#include <appl/globalMsg.hpp>
#include <ewol/widget/TreeView.hpp>
#include <ewol/widget/Windows.hpp>
#include <etk/FlatTree.hpp>
#include <etk/path/fileSystem.hpp>

namespace appl {
	class TreeElement {
		public:
			TreeElement(const etk::Path& _path, bool _isExpand = false):
			  m_path(_path),
			  m_nodeName(_path.getFileName()),
			  m_isFolder(etk::path::isDirectory(_path) == true),
			  m_buffer(null),
			  m_isSelected(false),
			  m_isExpand(_isExpand) {
				m_haveChild = etk::path::haveChild(_path);
			}
			etk::Path m_path;
			etk::String m_nodeName; // must be here ==> the buffer is optionnal..
			bool m_isFolder;
			bool m_haveChildOpen = false;
			bool m_haveChild = false;
			ememory::SharedPtr<appl::Buffer> m_buffer;
			bool m_isSelected = false;
			bool m_isExpand = false;
	};
	namespace widget {
		
		class BufferTree;
		using BufferTreeShared = ememory::SharedPtr<BufferTree>;
		using BufferTreeWeak = ememory::WeakPtr<BufferTree>;
		
		class BufferTree : public ewol::widget::TreeView {
			public:
				eproperty::Value<bool> propertyShowUnNeeded; //!< Size of the icon.
			private:
				ememory::SharedPtr<appl::BufferManager> m_bufferManager; //!< handle on the buffer manager
			private:
				ememory::SharedPtr<appl::GlyphPainting> m_paintingProperties; //!< element painting property
				int32_t m_colorBackground1;
				int32_t m_colorBackground2;
				int32_t m_colorBackgroundHide;
				int32_t m_colorBackgroundSelect;
				int32_t m_colorTextNormal;
				int32_t m_colorTextModify;
				int32_t m_colorTextNotOpen;
			private:
				int32_t m_selectedIdRequested;
				int32_t m_selectedID;
				void updateFlatTree();
				void generateFlatTree();
				// Recursive update of the child opening or not
				bool updateChildOpen(ememory::SharedPtr<etk::TreeNode<appl::TreeElement>> _node);
				etk::Path getRootPath();
				void populateNodeIfNeeded(ememory::SharedPtr<etk::TreeNode<appl::TreeElement>> _node);
				void expandToPath(ememory::SharedPtr<etk::TreeNode<appl::TreeElement>> _node, const etk::Path& _path);
				void goUpper();
				ememory::SharedPtr<etk::TreeNode<TreeElement>> m_tree;
				etk::FlatTree<TreeElement> m_flatTree;
				ememory::SharedPtr<appl::Buffer> m_selection;
			protected:
				// Constructeur
				BufferTree();
				void init() override;
			public:
				DECLARE_FACTORY(BufferTree);
				virtual ~BufferTree();
			private:
				bool m_openOrderMode; //!< true if the order is the opening order mode, otherwise, Alphabetic order
			protected:
				// function call to display the list :
				etk::Color<> getBasicBG() override;
				void removeAllElement();
				// Derived function
				ivec2 getMatrixSize() const override;
				fluorine::Variant getData(int32_t _role, const ivec2& _pos) override;
				bool onItemEvent(const ewol::event::Input& _event, const ivec2& _pos, const vec2& _mousePosition) override;
				void onItemExpandEvent(const ivec2& _pos) override;
			private: //callback function:
				void onCallbackChangeName();
				void onCallbackIsSave();
				void onCallbackIsModify();
				void onNewBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer);
				void onSelectBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer);
				void onRemoveBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer);
				void calculateMinMaxSize() override;
				void onChangePropertyShowUnNeeded();
		};
	}
}
