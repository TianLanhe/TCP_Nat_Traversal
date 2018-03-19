#ifndef SMART_POINTER_H
#define SMART_POINTER_H

#include "Exception.h"

namespace Lib {

	template<typename T>
	class SmartPointer {
	public:
		explicit SmartPointer(T* p = nullptr) { _attach(p); }
		SmartPointer(const SmartPointer<T>& sp);

		~SmartPointer() { _release(); }

		void swap(SmartPointer<T>& sp);
		void reset(T* p = nullptr);

		T* get() const { return m_pointer; }
		size_t use_count() const { return m_counter ? *m_counter : 0; }

		SmartPointer<T>& operator=(const SmartPointer<T>& sp);
		T* operator->() const { return get(); }
		T& operator*() const { return *get(); }
		bool unique() const { return use_count() == 1; }
		explicit operator bool() const { return get() != nullptr; }


	private:
		T *m_pointer;
		size_t *m_counter;

		void _release();
		void _attach(T *p);
	};

	template<typename T>
	void SmartPointer<T>::_attach(T *p) {
		if (p) {
			m_counter = new size_t(1);
			if (m_counter == nullptr)
				THROW_EXCEPTION(NoMemoryException, "no memory to create reference counter.");
		}
		else {
			m_counter = nullptr;
		}
		m_pointer = p;
	}

	template<typename T>
	void SmartPointer<T>::_release() {
		if (m_pointer) {
			--*m_counter;
			if (*m_counter == 0) {
				delete m_pointer;
				delete m_counter;
			}
			m_pointer = nullptr;
			m_counter = nullptr;
		}
	}

	template<typename T>
	SmartPointer<T>::SmartPointer(const SmartPointer<T>& sp) {
		m_pointer = sp.m_pointer;
		m_counter = sp.m_counter;
		if (m_counter)
			++*m_counter;
	}

	template<typename T>
	void SmartPointer<T>::swap(SmartPointer<T>& sp) {
		if (sp.m_pointer != m_pointer) {
			T *temp = m_pointer;
			m_pointer = sp.m_pointer;
			sp.m_pointer = temp;

			size_t *tmp = m_counter;
			m_counter = sp.m_counter;
			sp.m_counter = tmp;
		}
	}

	template<typename T>
	void SmartPointer<T>::reset(T* p = nullptr) {
		_release();
		_attach(p);
	}

	template<typename T>
	SmartPointer<T>& SmartPointer<T>::operator=(const SmartPointer<T>& sp) {
		if (this != &sp) {
			_release();
			m_pointer = sp.m_pointer;
			m_counter = sp.m_counter;
			if (m_counter)
				++*m_counter;
		}
		return *this;
	}

}

#endif // !SMART_POINTER_H
