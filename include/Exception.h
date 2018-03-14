#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <cstddef>

namespace Lib {

#define THROW_EXCEPTION(e,message) (throw e(message,__FILE__,__LINE__))

	// 异常类基类，抽象类
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

	// 空指针异常
	class NullPointerException : public Exception {
	public:
		NullPointerException() {}
		explicit NullPointerException(const char* message) :Exception(message) {}
		NullPointerException(const char* message, const char* file, int line) :Exception(message, file, line) {}

	protected:
		virtual const char* getName();
	};

	// 参数错误异常
	class InvalidParameterException : public Exception {
	public:
		InvalidParameterException() {}
		explicit InvalidParameterException(const char* message) :Exception(message) {}
		InvalidParameterException(const char* message, const char* file, int line) :Exception(message, file, line) {}

	protected:
		virtual const char* getName();
	};

	// 越界异常
	class IndexOutOfBoundsException : public Exception {
	public:
		IndexOutOfBoundsException() {}
		explicit IndexOutOfBoundsException(const char* message) :Exception(message) {}
		IndexOutOfBoundsException(const char* message, const char* file, int line) :Exception(message, file, line) {}

	protected:
		virtual const char* getName();
	};

	// 内存不足异常
	class NoMemoryException : public Exception {
	public:
		NoMemoryException() {}
		explicit NoMemoryException(const char* message) :Exception(message) {}
		NoMemoryException(const char* message, const char* file, int line) :Exception(message, file, line) {}

	protected:
		virtual const char* getName();
	};

	// 非法操作异常
	class InvalidOperationException : public Exception {
	public:
		InvalidOperationException() {}
		explicit InvalidOperationException(const char* message) :Exception(message) {}
		InvalidOperationException(const char* message, const char* file, int line) :Exception(message, file, line) {}
	
	protected:
		virtual const char* getName();
	};

	// 非法状态异常
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
