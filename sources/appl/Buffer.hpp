/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once

#include <etk/types.hpp>
#include <ewol/ewol.hpp>
#include <etk/Buffer.hpp>
#include <ewol/object/Object.hpp>
#include <ewol/widget/Widget.hpp>
#include <ewol/compositing/Text.hpp>
#include <appl/Highlight.hpp>
#include <esignal/Signal.hpp>

namespace appl {

	class DisplayHLData {
		public:
			etk::Vector<appl::HighlightInfo> HLData;
			int64_t posHLPass1;
			int64_t posHLPass2;
	};
	class Buffer;
	using BufferShared = ememory::SharedPtr<appl::Buffer>;
	using BufferWeak = ememory::WeakPtr<appl::Buffer>;
	class Buffer : public ewol::Object {
		public:
			class Iterator {
				// Private data :
				private:
					int64_t m_current; //!< curent Id in the Buffer
					appl::Buffer* m_data; //!< Pointer on the curent Buffer
					char32_t m_value; //!< store vlue to prevent multiple calcule of getting the data
				public:
					/**
					 * @brief Basic itarator constructor with no link.
					 */
					Iterator():
					  m_current(0),
					  m_data(null),
					  m_value(u32char::Null) {
						// nothing to do ...
					};
					/**
					 * @brief Recopy constructor.
					 * @param[in] _obj The Iterator that might be copy
					 */
					Iterator(const Iterator & _obj):
					  m_current(_obj.m_current),
					  m_data(_obj.m_data),
					  m_value(u32char::Null) {
						// nothing to do ...
					};
					/**
					 * @brief Asignation operator.
					 * @param[in] _otherIterator The Iterator that might be copy
					 * @return reference on the curent Iterator
					 */
					Iterator& operator=(const Iterator & _obj) {
						m_current = _obj.m_current;
						m_data = _obj.m_data;
						m_value = u32char::Null;
						return *this;
					};
					/**
					 * @brief Basic destructor
					 */
					virtual ~Iterator() {
						m_current = 0;
						m_data = null;
						m_value = u32char::Null;
					};
					/**
					 * @brief basic boolean cast
					 * @return true if the element is present in buffer
					 */
					operator bool () const {
						if (m_data == null) {
							return false;
						}
						if (m_current >= (int64_t)m_data->m_data.size()) {
							return false;
						}
						if (m_current < 0) {
							return false;
						}
						return true;
					};
					/**
					 * @brief basic boolean cast
					 * @return true if the element is present in buffer
					 */
					operator int64_t () const {
						if (m_data == null) {
							return 0;
						}
						if (m_current < 0) {
							return 0;
						}
						if (m_current > (int64_t)m_data->m_data.size()) {
							return (int64_t)m_data->m_data.size();
						}
						return m_current;
					};
					/**
					 * @brief Incremental operator
					 * @return Reference on the current iterator incremented
					 */
					Iterator& operator++ ();
					/**
					 * @brief Decremental operator
					 * @return Reference on the current iterator decremented
					 */
					Iterator& operator-- ();
					/**
					 * @brief Incremental operator
					 * @return Reference on a new iterator and increment the other one
					 */
					Iterator operator++ (int32_t) {
						Iterator it(*this);
						++(*this);
						return it;
					};
					/**
					 * @brief Decremental operator
					 * @return Reference on a new iterator and decrement the other one
					 */
					Iterator operator-- (int32_t) {
						Iterator it(*this);
						--(*this);
						return it;
					};
					/**
					 * @brief egality iterator
					 * @return true if the iterator is identical pos
					 */
					bool operator== (const Iterator& _obj) const {
						if (    m_current == _obj.m_current
						     && m_data == _obj.m_data) {
							return true;
						}
						return false;
					};
					/**
					 * @brief egality iterator
					 * @return true if the iterator is identical pos
					 */
					bool operator!= (const Iterator& _obj) const {
						if (    m_current != _obj.m_current
						     || m_data != _obj.m_data) {
							return true;
						}
						return false;
					};
					/**
					 * @brief <= iterator
					 * @return true if the iterator is identical pos
					 */
					bool operator<= (const Iterator& _obj) const {
						if (m_data != _obj.m_data) {
							return false;
						}
						if (m_current <= _obj.m_current) {
							return true;
						}
						return false;
					};
					/**
					 * @brief >= iterator
					 * @return true if the iterator is identical pos
					 */
					bool operator>= (const Iterator& _obj) const {
						if (m_data != _obj.m_data) {
							return false;
						}
						if (m_current >= _obj.m_current) {
							return true;
						}
						return false;
					};
					/**
					 * @brief < iterator
					 * @return true if the iterator is identical pos
					 */
					bool operator< (const Iterator& _obj) const {
						if (m_data != _obj.m_data) {
							return false;
						}
						if (m_current < _obj.m_current) {
							return true;
						}
						return false;
					};
					/**
					 * @brief > iterator
					 * @return true if the iterator is identical pos
					 */
					bool operator> (const Iterator& _obj) const {
						if (m_data != _obj.m_data) {
							return false;
						}
						if (m_current > _obj.m_current) {
							return true;
						}
						return false;
					};
					/**
					 * @brief Get the value on the current element
					 * @return The request element value
					 */
					char32_t operator* ();
					/**
					 * @brief Get the position in the buffer
					 * @return The requested position.
					 */
					int64_t getPos() const {
						if (m_data == null) {
							return 0;
						}
						if (m_current < 0) {
							return 0;
						}
						if (m_current >= (int64_t)m_data->m_data.size()) {
							return m_data->m_data.size()-1;
						}
						return m_current;
					};
					/**
					 * @brief move the element position
					 * @return a new iterator.
					 */
					Iterator operator+ (const int64_t _val) const {
						Iterator tmpp(*this);
						for (int64_t iii=0; iii<_val; ++iii) {
							++tmpp;
						}
						return tmpp;
					};
					Iterator operator+ (const int32_t _val) const {
						Iterator tmpp(*this);
						for (int64_t iii=0; iii<_val; ++iii) {
							++tmpp;
						}
						return tmpp;
					};
					Iterator operator+ (const size_t _val) const {
						Iterator tmpp(*this);
						for (int64_t iii=0; iii<(int64_t)_val; ++iii) {
							++tmpp;
						}
						return tmpp;
					};
					/**
					 * @brief move the element position
					 * @return a new iterator.
					 */
					Iterator operator- (const int64_t _val) const {
						Iterator tmpp(*this);
						for (int64_t iii=0; iii<_val; ++iii) {
							--tmpp;
						}
						return tmpp;
					};
					Iterator operator- (const int32_t _val) const {
						Iterator tmpp(*this);
						for (int64_t iii=0; iii<_val; ++iii) {
							--tmpp;
						}
						return tmpp;
					};
					Iterator operator- (const size_t _val) const {
						Iterator tmpp(*this);
						for (int64_t iii=0; iii<(int64_t)_val; ++iii) {
							--tmpp;
						}
						return tmpp;
					};
				private:
					Iterator(Buffer* _obj, int64_t _pos) :
					  m_current(_pos),
					  m_data(_obj),
					  m_value(u32char::Null) {
						// nothing to do ...
					};
					friend class Buffer;
			};
		public: // signals
			esignal::Signal<> signalIsModify;
			esignal::Signal<> signalIsSave;
			esignal::Signal<> signalSelectChange;
			esignal::Signal<> signalChangeName;
		protected:
			Buffer();
			void init();
		public:
			DECLARE_FACTORY(Buffer);
			virtual ~Buffer();
		private:
			bool m_hasFileName; //!< When new file, the buffer has no name ==> but it might be reference with a single name ...
			etk::Path m_fileName; //!< name of the file (with his path)
		public:
			/**
			 * @brief get the curent filename of the Buffer
			 */
			const etk::Path& getFileName() const {
				return m_fileName;
			}
			/**
			 * @brief Check if the buffer has a real filename.
			 * @return the status of the existance of a name.
			 */
			bool hasFileName() const {
				return m_hasFileName;
			}
			/**
			 * @brief Load A specific file in this buffer.
			 * @param[in] _name name of the file.
			 * @return true if file corectly opened.
			 */
			bool loadFile(const etk::Path& _name);
			/**
			 * @brief Set a file name at this buffer (no saving ...)
			 * @param[in] _name name of the file.
			 */
			void setFileName(const etk::Path& _name);
			/**
			 * @brief save the file in the specify path.
			 * @return true is saving well done
			 */
			bool storeFile();
		protected:
			bool m_isModify; //!< true if the file is modify
		public:
			/**
			 * @breif get the status of file modification.
			 * @return true if file is modify.
			 */
			bool isModify() const {
				return m_isModify;
			}
			/**
			 * @brief Set the file modification status.
			 * @param[in] _status New modification status.
			 */
			void setModification(bool _status);
		protected:
			etk::Buffer m_data; //!< copy of the file buffer
		public:
			etk::Buffer& getData() {
				return m_data;
			};
		protected:
			int64_t m_cursorPos; //!< cursor position.
		public:
			void moveCursor(int64_t _pos);
		protected:
			int64_t m_cursorSelectPos; //!< cursor position.
		public:
			/**
			 * @brief Set the selection position in the buffer.
			 * @param[in] _pos Position of the selection.
			 */
			void setSelectionPos(const Iterator& _pos);
			/**
			 * @brief Remove Selection of the buffer.
			 */
			void unSelect();
			/**
			 * @brief Remove the selection of the buffer. (do nothing if no secection)
			 */
			void removeSelection();
			/**
			 * @brief Get the status of selection.
			 * @return true if we have a curent selection, false otherwise.
			 */
			bool hasTextSelected() {
				return m_cursorSelectPos >= 0;
			}
			/**
			 * @brief Get the Start position of the selection.
			 * @return position of the start selection.
			 */
			int64_t getStartSelectionPos() {
				return etk::min(m_cursorPos, m_cursorSelectPos);
			}
			/**
			 * @brief Get the Stop position of the selection.
			 * @return position of the stop selection.
			 */
			int64_t getStopSelectionPos() {
				return etk::max(m_cursorPos, m_cursorSelectPos);
			}
		protected:
			float m_cursorPreferredCol; //!< position of the cursor when up and down is done.
		public:
			/**
			 * @brief Set the favorite up and down position (distance from the left of the screen.
			 * @param[in] _val New distance (in pixels).
			 */
			void setFavoriteUpDownPos(float _val) {
				m_cursorPreferredCol = _val;
			}
			/**
			 * @brief Get the favorite distance from the left screen (For up and down moving).
			 * @return The distance in pixels.
			 */
			float getFavoriteUpDownPos() {
				return m_cursorPreferredCol;
			}
		protected:
			bool m_selectMode; //!< when true, the select mode keep the moving selecting
		public:
			/**
			 * @brief Set the selection mode (if true, the move event creata a selection)
			 * @param[in] _status New status of the section.
			 */
			void setSelectMode(bool _status) {
				m_selectMode = _status;
			}
			/**
			 * @brief Get the selection mode (if true, the move event creata a selection)
			 * @return The selecting mode.
			 */
			bool getSelectMode() {
				return m_selectMode;
			}
		public:
			/**
			 * @brief Get the position of selection around (select word).
			 * @param[in] _startPos Position to start the selection.
			 * @param[out] _beginPos Position where the element start.
			 * @param[out] _endPos Position where the element stop.
			 * @return true if we find a selection around.
			 */
			bool getPosAround(const Iterator& _startPos, Iterator &_beginPos, Iterator &_endPos);
			/**
			 * @brief Expand the specify char to have a user frendly display for special char and tabs
			 * @param[in] _indent Curent indentation in the line
			 * @param[in] _value Current value to transform
			 * @param[out] _out String that represent the curent value to display
			 */
			void expand(int32_t& _indent, const char32_t& _value, etk::UString& _out) const;
			/**
			 * @brief get the start of a line with the position in the buffer.
			 * @param[in] _pos position in the buffer.
			 * @return The position in the buffer of the start of the line.
			 */
			Iterator getStartLine(const Iterator& _pos);
			/**
			 * @brief get the end of a line with the position in the buffer.
			 * @param[in] _pos position in the buffer.
			 * @return The position in the buffer of the end of the line.
			 */
			Iterator getEndLine(const Iterator& _pos);
			/**
			 * @brief Search a character in the buffer.
			 * @param[in] _pos Position to start the search of the element.
			 * @param[in] _search Character to search.
			 * @param[out] _result Research position.
			 * @return true if pos if fined.
			 */
			// TODO : rename find
			bool search(const Iterator& _pos, const char32_t& _search, Iterator& _result);
			/**
			 * @brief Search a character in the buffer in back mode.
			 * @param[in] _pos Position to start the search of the element.
			 * @param[in] _search Character to search.
			 * @param[out] _result Research position.
			 * @return true if pos if fined.
			 */
			// TODO : rename rfind
			bool searchBack(const Iterator& _pos, const char32_t& _search, Iterator& _result);
			/**
			 * @brief Search a string in the buffer.
			 * @param[in] _pos Position to start the search of the element.
			 * @param[in] _search String to search.
			 * @param[out] _result Research position.
			 * @param[in] _caseSensitive (optional) Search making attention of the case [default true]
			 * @return true if pos if fined.
			 */
			// TODO : rename find
			bool search(const Iterator& _pos, const etk::UString& _search, Iterator& _result, bool _caseSensitive = true);
			/**
			 * @brief Search a string in the buffer in back mode.
			 * @param[in] _pos Position to start the search of the element.
			 * @param[in] _search String to search.
			 * @param[out] _result Research position.
			 * @param[in] _caseSensitive (optional) Search making attention of the case [default true]
			 * @return true if pos if fined.
			 */
			// TODO : rename rfind
			bool searchBack(const Iterator& _pos, const etk::UString& _search, Iterator& _result, bool _caseSensitive = true);
			/**
			 * @brief find the first character of the line "nLines" forward
			 * @param[in] _startPos Start position.
			 * @param[in] _nLines Number of line to count.
			 * @return position of the starting the line.
			 */
			Iterator countForwardNLines(const Iterator& _startPos, int32_t _nLines);
			/**
			 * @brief find the first character of the line "nLines" backwards
			 * @param[in] _startPos Start position to count (this caracter is not counted)
			 * @param[in] _nLines Number of line to count (if  == 0 means find the beginning of the line)
			 * @return position of the starting the line
			 */
			Iterator countBackwardNLines(const Iterator& _startPos, int32_t _nLines);
		public:
			/**
			 * @brief copy data in the _data ref value.
			 * @param[out] _data Output stream to copy.
			 * @return true of no error occured.
			 */
			bool copy(etk::String& _data);
			/**
			 * @brief copy data in the _data ref value.
			 * @param[out] _data Output stream to copy.
			 * @param[in] _pos Position to add the data.
			 * @param[in] _posEnd End position to end replace the data.
			 */
			void copy(etk::String& _data, const appl::Buffer::Iterator& _pos, const appl::Buffer::Iterator& _posEnd);
			/**
			 * @brief Write data at a specific position
			 * @param[in] _data Data to insert in the buffer
			 * @param[in] _pos Position to add the data.
			 * @return true if the write is done corectly
			 */
			bool write(const etk::String& _data, const appl::Buffer::Iterator& _pos);
			/**
			 * @brief Write data at a specific position
			 * @param[in] _data Data to insert in the buffer
			 * @param[in] _pos Position to add the data.
			 * @param[in] _posEnd End position to end replace the data.
			 * @return true if the write is done corectly
			 */
			bool replace(const etk::String& _data, const appl::Buffer::Iterator& _pos, const appl::Buffer::Iterator& _posEnd);
		public: // iterator section :
			/**
			 * @brief Get an iterator an an specific position
			 * @param[in] _pos Requested position of the iterator.
			 * @return The Iterator
			 */
			Iterator position(int64_t _pos);
			/**
			 * @brief Get an Iterator on the start position.
			 * @return The Iterator
			 */
			Iterator begin();
			/**
			 * @brief Get an Iterator on the end position.
			 * @return The Iterator
			 */
			Iterator end();
			/**
			 * @brief Get an Iterator on the cursor position.
			 * @return The Iterator
			 */
			Iterator cursor();
			/**
			 * @brief Get an Iterator on the start selection.
			 * @return The Iterator
			 */
			Iterator selectStart();
			/**
			 * @brief Get an Iterator on the stop selection.
			 * @return The Iterator
			 */
			Iterator selectStop();
		protected:
			int32_t m_nbLines; //!< number of line in the buffer
		public:
			/**
			 * @brief Get the number of line in the buffer.
			 * @return number of line in the Buffer.
			 */
			int32_t getNumberOfLines() {
				return m_nbLines;
			}
			/**
			 * @brief Get the cursor line numberin the buffer.
			 * @return the line id in the Buffer.
			 */
			uint32_t getCursorLinesId();
		protected:
			/**
			 * @brief Count the number of line in the buffer
			 */
			void countNumberofLine();
		protected:
			etk::String m_highlightType; //!< Name of the highlight type
			
			ememory::SharedPtr<appl::Highlight> m_highlight; //!< internal link with the Highlight system
			etk::Vector<appl::HighlightInfo> m_HLDataPass1; //!< colorisation position in the current buffer pass 1
		public:
			/**
			 * @brief Find the Highligh capability
			 */
			void tryFindHighlightType();
			/**
			 * @brief Set type of highlight
			 * @param[in] _type type of the highlight
			 */
			void setHighlightType(const etk::String& _type);
			/**
			 * @brief Get type of highlight
			 * @return Type of the highlight
			 */
			const etk::String& setHighlightType() {
				return m_highlightType;
			};
			
			void regenerateHighLightAt(int64_t _pos, int64_t _nbDeleted, int64_t _nbAdded);
			void findMainHighLightPosition(int64_t _startPos,
			                               int64_t _endPos,
			                               int64_t& _startId,
			                               int64_t& _stopId,
			                               bool _backPreviousNotEnded);
			void generateHighLightAt(int64_t _pos, int64_t _endPos, int64_t _addingPos=0);
			void cleanHighLight();
			appl::HighlightInfo* getElementColorAtPosition(int64_t _pos, int64_t &_starPos);
			void hightlightGenerateLines(appl::DisplayHLData& _MData, const appl::Buffer::Iterator& _HLStart, int64_t _nbLines);
			appl::HighlightInfo* getElementColorAtPosition(appl::DisplayHLData& _MData, int64_t _pos);
	};
}
