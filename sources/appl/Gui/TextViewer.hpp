/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once
#include <appl/debug.hpp>
#include <appl/Buffer.hpp>
#include <appl/globalMsg.hpp>

#include <ewol/widget/WidgetScrolled.hpp>
#include <ewol/compositing/Text.hpp>
#include <ewol/compositing/TextDF.hpp>
#include <ewol/compositing/Drawing.hpp>
#include <appl/BufferManager.hpp>
#include <appl/Gui/ViewerManager.hpp>

namespace appl {
	class textPluginManager;
	class TextViewer;
	using TextViewerShared = ememory::SharedPtr<appl::TextViewer>;
	using TextViewerWeak = ememory::WeakPtr<appl::TextViewer>;
	class TextViewer : public ewol::widget::WidgetScrolled {
		private:
			eproperty::Value<etk::String> propertyFontName; //!< name of the font to display text.
			eproperty::Value<int32_t> propertyFontSize; //!< Size of the font to display text.
			
			ememory::SharedPtr<appl::GlyphPainting> m_paintingProperties; //!< element painting property
			int32_t m_colorBackground;
			int32_t m_colorSpace;
			int32_t m_colorTabulation;
			int32_t m_colorCursor;
			int32_t m_colorLineNumber;
			int32_t m_colorSelection;
			int32_t m_colorNormal;
		private:
			ememory::SharedPtr<appl::BufferManager> m_bufferManager; //!< handle on the buffer manager
			ememory::SharedPtr<appl::textPluginManager> m_pluginManager; //!< Plugin manager interface
			ememory::SharedPtr<appl::ViewerManager> m_viewerManager; //!< handle on the buffer manager
		protected:
			TextViewer();
			void init();
		public:
			DECLARE_FACTORY(TextViewer);
			virtual ~TextViewer();
		private:
			ememory::SharedPtr<appl::Buffer> m_buffer; //!< pointer on the current buffer to display (can be null if the buffer is remover or in state of changing buffer)
		public:
			/**
			 * @brief Get the buffer property (only for the class : template <typename TYPE> class TextViewerPluginData)
			 * @return pointer on buffer
			 */
			ememory::SharedPtr<appl::Buffer> internalGetBuffer() {
				return m_buffer;
			}
		private:
			ewol::compositing::Text m_displayText; //!< Text display properties.
			ewol::compositing::Drawing m_displayDrawing; //!< Other display requested.
			etk::Vector<etk::Pair<ememory::WeakPtr<appl::Buffer>, vec2>> m_drawingRemenber;
		public:
			virtual void onChangePropertyFontSize();
			virtual void onChangePropertyFontName();
		protected: // derived function
			virtual void onDraw();
		public:  // Derived function
			virtual bool calculateMinSize();
			virtual void onRegenerateDisplay();
			virtual bool onEventInput(const ewol::event::Input& _event);
			virtual bool onEventEntry(const ewol::event::Entry& _event);
			virtual void onEventClipboard(enum gale::context::clipBoard::clipboardListe _clipboardID);
			virtual void onGetFocus();
			virtual void onLostFocus();
		private:
			float m_lastOffsetDisplay; //!< Line number ofssed in the display
		private:
			bool m_insertMode; //!< the insert mode is enable
		public:
			/**
			 * @brief Update the scrolling position from the cursor position,
			 * it might be be all time in the display screen.
			 */
			void updateScrolling();
			// TODO : Doc : write data on buffer
			bool moveCursor(const appl::Buffer::Iterator& _pos);
			bool write(const etk::String& _data);
			bool write(const etk::String& _data, const appl::Buffer::Iterator& _pos);
			bool replace(const etk::String& _data, const appl::Buffer::Iterator& _pos, const appl::Buffer::Iterator& _posEnd);
			bool replace(const etk::String& _data);
			bool replace(const etk::UString& _data) {
				return replace(etk::toString(_data));
			}
			/**
			 * @brief Remove selected data ...
			 */
			void remove();
			/**
			 * @brief Remove selected data ... (No plugin call)
			 */
			void removeDirect() {
				if (m_buffer==null) {
					return;
				}
				m_buffer->removeSelection();
				updateScrolling();
			}
			
			/**
			 * @brief copy data in the _data ref value.
			 * @param[out] _data Output stream to copy.
			 * @return true of no error occured.
			 */
			bool copy(etk::String& _data) {
				if (m_buffer==null) {
					return false;
				}
				return m_buffer->copy(_data);
			}
			/**
			 * @brief copy data in the _data ref value.
			 * @param[out] _data Output stream to copy.
			 * @param[in] _pos Position to add the data.
			 * @param[in] _posEnd End position to end replace the data.
			 */
			void copy(etk::String& _data, const appl::Buffer::Iterator& _pos, const appl::Buffer::Iterator& _posEnd) {
				if (m_buffer==null) {
					return;
				}
				m_buffer->copy(_data, _pos, _posEnd);
			}
			/**
			 * @brief Write data at a specific position (No plugin call)
			 * @param[in] _data Data to insert in the buffer
			 * @param[in] _pos Position to add the data.
			 * @return true if the write is done corectly
			 */
			bool writeDirect(const etk::String& _data, const appl::Buffer::Iterator& _pos) {
				if (m_buffer==null) {
					return false;
				}
				bool ret = m_buffer->write(_data, _pos);
				updateScrolling();
				return ret;
			}
			/**
			 * @brief Write data at a specific position (No plugin call)
			 * @param[in] _data Data to insert in the buffer
			 * @param[in] _pos Position to add the data.
			 * @param[in] _posEnd End position to end replace the data.
			 * @return true if the write is done corectly
			 */
			bool replaceDirect(const etk::String& _data, const appl::Buffer::Iterator& _pos, const appl::Buffer::Iterator& _posEnd) {
				if (m_buffer==null) {
					return false;
				}
				bool ret = m_buffer->replace(_data, _pos, _posEnd);
				updateScrolling();
				return ret;
			}
			
			appl::Buffer::Iterator getMousePosition(const vec2& _relativePos);
			void mouseEventDouble();
			void mouseEventTriple();
		private:
			enum moveMode {
				moveLetter,
				moveWord,
				moveEnd
			};
			/**
			 * Move the cursor right in the line (no stop of a new line)
			 * @param[in] _mode Moving mode char, word, ...
			 */
			void moveCursorRight(moveMode _mode = moveLetter);
			/**
			 * Move the cursor left in the line (no stop of a new line)
			 * @param[in] _mode Moving mode char, word, ...
			 */
			void moveCursorLeft(moveMode _mode = moveLetter);
			/**
			 * @brief Move the cursor at an other position upper.
			 * @param[in] _nbLine number of up line that might be moved
			 */
			void moveCursorUp(uint32_t _nbLine);
			/**
			 * @brief Move the cursor at an other position under.
			 * @param[in] _nbLine number of down line that might be moved
			 */
			void moveCursorDown(uint32_t _nbLine);
			
			appl::Buffer::Iterator getPosSize(const appl::Buffer::Iterator& _startLinePos, float _distance);
			float getScreenSize(const appl::Buffer::Iterator& _startLinePos, const appl::Buffer::Iterator& _stopPos);
		private:
			/**
			 * @brief Set the current buffer selected
			 */
			void setCurrentSelect();
			/**
			 * @brief Check if the current buffer is last selected
			 * @return true if selected last
			 */
			bool isSelectedLast();
		public:
			/**
			 * @brief get the path of the current buffer
			 * @return Path of the buffer (remove the ended name)
			 */
			virtual etk::String getBufferPath();
			/**
			 * @brief Check if the buffer is availlable
			 * @return true if a display buffer is present, false otherwise.
			 */
			virtual bool hasBuffer() {
				return m_buffer != null;
			}
			/**
			 * @brief Get the status of selection.
			 * @return true if we have a current selection, false otherwise.
			 */
			virtual bool hasTextSelected() {
				if (m_buffer==null) {
					return false;
				}
				return m_buffer->hasTextSelected();
			}
			/**
			 * @brief Remove Selection of the buffer.
			 */
			virtual void unSelect() {
				if (m_buffer==null) {
					return;
				}
				m_buffer->unSelect();
			}
			/**
			 * @brief Select a section of text.
			 * @param[in] _start Start position of the selection
			 * @param[in] _stop Stop position of the selection (the curor is set at this position)
			 */
			virtual void select(const appl::Buffer::Iterator& _start, const appl::Buffer::Iterator& _stop) {
				if (m_buffer==null) {
					return;
				}
				moveCursor(_stop);
				m_buffer->setSelectionPos(_start);
			}
			/**
			 * @brief Find a string in the buffer.
			 * @param[in] _pos Position to start the search of the element.
			 * @param[in] _search String to search.
			 * @param[out] _resultStart Find element start position.
			 * @param[out] _resultStop Find element end position.
			 * @param[in] _caseSensitive (optional) Search making attention of the case [default true]
			 * @return true if pos if fined.
			 */
			virtual bool find(const appl::Buffer::Iterator& _pos,
			                  const etk::UString& _search,
			                  appl::Buffer::Iterator& _resultStart,
			                  appl::Buffer::Iterator& _resultStop,
			                  bool _caseSensitive = true) {
				if (m_buffer==null) {
					return false;
				}
				bool ret = m_buffer->search(_pos, _search, _resultStart, _caseSensitive);
				if (ret == true) {
					_resultStop = _resultStart + _search.size();
				}
				return ret;
			}
			/**
			 * @brief revers find a string in the buffer.
			 * @param[in] _pos Position to start the search of the element.
			 * @param[in] _search String to search.
			 * @param[out] _resultStart Find element start position.
			 * @param[out] _resultStop Find element end position.
			 * @param[in] _caseSensitive (optional) Search making attention of the case [default true]
			 * @return true if pos if fined.
			 */
			virtual bool rfind(const appl::Buffer::Iterator& _pos,
			                   const etk::UString& _search,
			                   appl::Buffer::Iterator& _resultStart,
			                   appl::Buffer::Iterator& _resultStop,
			                   bool _caseSensitive = true) {
				if (m_buffer==null) {
					return false;
				}
				bool ret = m_buffer->searchBack(_pos, _search, _resultStart, _caseSensitive);
				if (ret == true) {
					_resultStop = _resultStart + _search.size();
				}
				return ret;
			}
			/**
			 * @brief Get the position of selection around (select word).
			 * @param[in] _pos Position to start the selection.
			 * @param[out] _beginPos Position where the element start.
			 * @param[out] _endPos Position where the element stop.
			 * @return true if we find a selection around.
			 */
			bool getPosAround(const appl::Buffer::Iterator& _pos,
			                  appl::Buffer::Iterator &_beginPos,
			                  appl::Buffer::Iterator &_endPos) {
				if (m_buffer==null) {
					return false;
				}
				return m_buffer->getPosAround(_pos, _beginPos, _endPos);
			}
			/**
			 * @brief Get an iterator an an specific position
			 * @param[in] _pos Requested position of the iterator.
			 * @return The Iterator
			 */
			appl::Buffer::Iterator position(int64_t _pos) {
				if (m_buffer==null) {
					return appl::Buffer::Iterator();
				}
				return m_buffer->position(_pos);
			}
			/**
			 * @brief Get the cursor position.
			 * @return The iterator on the cursor position
			 */
			appl::Buffer::Iterator cursor() {
				if (m_buffer==null) {
					return appl::Buffer::Iterator();
				}
				return m_buffer->cursor();
			}
			/**
			 * @brief Get the begin position.
			 * @return The iterator on the begin position
			 */
			appl::Buffer::Iterator begin() {
				if (m_buffer==null) {
					return appl::Buffer::Iterator();
				}
				return m_buffer->begin();
			}
			/**
			 * @brief Get the end position.
			 * @return The iterator on the end position
			 */
			appl::Buffer::Iterator end() {
				if (m_buffer==null) {
					return appl::Buffer::Iterator();
				}
				return m_buffer->end();
			}
			/**
			 * @brief Get an Iterator on the start selection.
			 * @return The Iterator
			 */
			appl::Buffer::Iterator selectStart() {
				if (m_buffer==null) {
					return appl::Buffer::Iterator();
				}
				return m_buffer->selectStart();
			}
			/**
			 * @brief Get an Iterator on the stop selection.
			 * @return The Iterator
			 */
			appl::Buffer::Iterator selectStop() {
				if (m_buffer==null) {
					return appl::Buffer::Iterator();
				}
				return m_buffer->selectStop();
			}
			/**
			 * @brief get the start of a line with the position in the buffer.
			 * @param[in] _pos position in the buffer.
			 * @return The position in the buffer of the start of the line.
			 */
			appl::Buffer::Iterator getStartLine(const appl::Buffer::Iterator& _pos) {
				if (m_buffer==null) {
					return appl::Buffer::Iterator();
				}
				return m_buffer->getStartLine(_pos);
			}
			/**
			 * @brief get the end of a line with the position in the buffer.
			 * @param[in] _pos position in the buffer.
			 * @return The position in the buffer of the end of the line.
			 */
			appl::Buffer::Iterator getEndLine(const appl::Buffer::Iterator& _pos) {
				if (m_buffer==null) {
					return appl::Buffer::Iterator();
				}
				return m_buffer->getEndLine(_pos);
			}
			/**
			 * @brief Register of the arrival of a Multicast message
			 * @param[in] _messageId Event Id waiting for...
			 */
			// TODO : Remove
			void ext_registerMultiCast(const char* const _messageId) {
				//registerMultiCast(_messageId);
			}
			/**
			 * @brief add a specific shortcut with his description
			 * @param[in] _descriptiveString Description string of the shortcut
			 * @param[in] _generateEventName Event generic of the element
			 */
			virtual void ext_shortCutAdd(const etk::String& _descriptiveString,
			                             const etk::String& _generateEventName) {
				shortCutAdd(_descriptiveString, _generateEventName);
			}
			/**
			 * @brief Remove a specific shortcut with his event name
			 * @param[in] _generateEventName Event of the element shortcut
			 */
			virtual void ext_shortCutRm(const etk::String& _generateEventName) {
				shortCutRemove(_generateEventName);
			}
		private: // callback fundtions
			void onCallbackIsModify();
			void onCallbackShortCut(const etk::String& _value);
			void onCallbackSelectChange();
			void onCallbackselectNewFile(const ememory::SharedPtr<appl::Buffer>& _value);
	};
}

