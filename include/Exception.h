#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <cstddef>

namespace Lib {

#define THROW_EXCEPTION(e,message) (throw e(message,__FILE__,__LINE__))

	// �쳣����࣬������
	class Exception {
	public:
		Exception(const char* mssage = NULL);
		Exception(const char* message, const char* file, int line);
		Exception(const Exception&);

		virtual ~Exception() = 0;

		Exception& operator= (const Exception&);

		virtual const char* message();
		virtual const char* location();

	protected:
		char *m_message;
		char *m_location;

		virtual const char* getName();
	};

	// ��ָ���쳣
	class NullPointerException : public Exception {
	public:
		NullPointerException() {}
		explicit NullPointerException(const char* message) :Exception(message) {}
		NullPointerException(const char* message, const char* file, int line) :Exception(message, file, line) {}

	protected:
		virtual const char* getName();
	};

	// ���������쳣
	class InvalidParameterException : public Exception {
	public:
		InvalidParameterException() {}
		explicit InvalidParameterException(const char* message) :Exception(message) {}
		InvalidParameterException(const char* message, const char* file, int line) :Exception(message, file, line) {}

	protected:
		virtual const char* getName();
	};

	// Խ���쳣
	class IndexOutOfBoundsException : public Exception {
	public:
		IndexOutOfBoundsException() {}
		explicit IndexOutOfBoundsException(const char* message) :Exception(message) {}
		IndexOutOfBoundsException(const char* message, const char* file, int line) :Exception(message, file, line) {}

	protected:
		virtual const char* getName();
	};

	// �ڴ治���쳣
	class NoMemoryException : public Exception {
	public:
		NoMemoryException() {}
		explicit NoMemoryException(const char* message) :Exception(message) {}
		NoMemoryException(const char* message, const char* file, int line) :Exception(message, file, line) {}

	protected:
		virtual const char* getName();
	};

	// �Ƿ������쳣
	class InvalidOperationException : public Exception {
	public:
		InvalidOperationException() {}
		explicit InvalidOperationException(const char* message) :Exception(message) {}
		InvalidOperationException(const char* message, const char* file, int line) :Exception(message, file, line) {}
	
	protected:
		virtual const char* getName();
	};

	// �Ƿ�״̬�쳣
	class ErrorStateException : public Exception {
	public:
		ErrorStateException() {}
		explicit ErrorStateException(const char* message) :Exception(message) {}
		ErrorStateException(const char* message, const char* file, int line) :Exception(message, file, line) {}

	protected:
		virtual const char* getName();
	};
}

#endif // !EXCEPTION_H
