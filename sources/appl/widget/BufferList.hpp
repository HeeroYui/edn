/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once

#include <appl/debug.hpp>
#include <appl/BufferManager.hpp>
#include <appl/globalMsg.hpp>
#include <ewol/widget/List.hpp>
#include <ewol/widget/Windows.hpp>

namespace appl {
	class dataBufferStruct {
		public:
			etk::Path m_bufferName;
			ememory::SharedPtr<appl::Buffer> m_buffer;
			dataBufferStruct() {
				
			};
			dataBufferStruct(const etk::Path& _bufferName, const ememory::SharedPtr<appl::Buffer>& _buffer) :
			  m_bufferName(_bufferName),
			  m_buffer(_buffer) {
				
			};
			virtual ~dataBufferStruct() { };
	};
};

namespace appl {
	namespace widget {
		class BufferList;
		using BufferListShared = ememory::SharedPtr<BufferList>;
		using BufferListWeak = ememory::WeakPtr<BufferList>;
		
		class BufferList : public ewol::widget::List {
			private:
				ememory::SharedPtr<appl::BufferManager> m_bufferManager; //!< handle on the buffer manager
			private:
				ememory::SharedPtr<appl::GlyphPainting> m_paintingProperties; //!< element painting property
				int32_t m_colorBackground1;
				int32_t m_colorBackground2;
				int32_t m_colorBackgroundSelect;
				int32_t m_colorTextNormal;
				int32_t m_colorTextModify;
			private:
				int32_t m_selectedIdRequested;
				int32_t m_selectedID;
				etk::Vector<appl::dataBufferStruct> m_list;
				/**
				 * @brief Insert the element in the alphabetic order.
				 * @param[in] _dataStruct element to add.
				 */
				void insertAlphabetic(const appl::dataBufferStruct& _dataStruct, bool _selectNewPosition = false);
			protected:
				// Constructeur
				BufferList();
				void init() override;
			public:
				DECLARE_FACTORY(BufferList);
				virtual ~BufferList();
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
			private: //callback function:
				void onCallbackChangeName();
				void onCallbackIsSave();
				void onCallbackIsModify();
				void onCallbackNewBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer);
				void onCallbackselectNewFile(const ememory::SharedPtr<appl::Buffer>& _buffer);
				void onCallbackBufferRemoved(const ememory::SharedPtr<appl::Buffer>& _buffer);
				void calculateMinMaxSize() override;
		};
	}
}
