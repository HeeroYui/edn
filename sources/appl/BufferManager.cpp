/**
 * @author Edouard DUPIN
 * 
 * @copyright 2010, Edouard DUPIN, all right reserved
 * 
 * @license GPL v3 (see license file)
 */

#include <etk/types.h>
#include <etk/os/FSNode.h>
#include <appl/debug.h>
#include <appl/global.h>
#include <appl/BufferManager.h>
#include <ewol/object/Object.h>
#include <ewol/object/Manager.h>
#include <ewol/resource/Manager.h>

#undef __class__
#define __class__ "BufferManager"

appl::BufferManager::BufferManager() {
	addObjectType("appl::BufferManager");
}

void appl::BufferManager::init(const std::string& _uniqueName) {
	ewol::Resource::init(_uniqueName);
}

appl::BufferManager::~BufferManager() {
	m_list.clear();
}


std::shared_ptr<appl::Buffer> appl::BufferManager::createNewBuffer() {
	std::shared_ptr<appl::Buffer> tmp = appl::Buffer::create();
	if (tmp == nullptr) {
		APPL_ERROR("Can not allocate the Buffer (empty).");
		return nullptr;
	}
	m_list.push_back(tmp);
	sendMultiCast(appl::MsgSelectNewFile, tmp->getFileName());
	return tmp;
}

std::shared_ptr<appl::Buffer> appl::BufferManager::get(const std::string& _fileName, bool _createIfNeeded) {
	APPL_INFO("get(" << _fileName << "," << _createIfNeeded << ")");
	for (auto &it : m_list) {
		if (it == nullptr) {
			continue;
		}
		if (it->getFileName() == _fileName) {
			return it;
		}
	}
	if (_createIfNeeded == true) {
		if (etk::FSNodeGetType(_fileName) == etk::FSN_FOLDER) {
			APPL_WARNING("try open a folder : " << _fileName);
			APPL_CRITICAL("plop");
			return nullptr;
		}
		std::shared_ptr<appl::Buffer> tmp = appl::Buffer::create();
		if (tmp == nullptr) {
			APPL_ERROR("Can not allocate the Buffer class : " << _fileName);
			return nullptr;
		}
		tmp->loadFile(_fileName);
		m_list.push_back(tmp);
		return tmp;
	}
	return nullptr;
}
void appl::BufferManager::setBufferSelected(std::shared_ptr<appl::Buffer> _bufferSelected) {
	m_bufferSelected = _bufferSelected;
	sendMultiCast(appl::MsgSelectChange, "");
}

std::shared_ptr<appl::Buffer> appl::BufferManager::get(int32_t _id) {
	int32_t id = 0;
	for (auto &it : m_list) {
		if (id == _id) {
			return it;
		}
		id++;
	}
	return m_list.back();
}

bool appl::BufferManager::exist(const std::string& _fileName) {
	for (auto &it : m_list) {
		if (it == nullptr) {
			continue;
		}
		if (it->getFileName() == _fileName) {
			return true;
		}
	}
	return false;
}

void appl::BufferManager::open(const std::string& _fileName) {
	if (exist(_fileName) == true) {
		return;
	}
	if (get(_fileName, true) == nullptr) {
		return;
	}
	sendMultiCast(appl::MsgSelectNewFile, _fileName);
}

void appl::BufferManager::onReceiveMessage(const ewol::object::Message& _msg) {
	APPL_DEBUG("receive message !!! " << _msg);
}

