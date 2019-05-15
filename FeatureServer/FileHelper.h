//
// Created by test2 on 19-5-15.
//

#ifndef MUDUO_FILEHELPER_H
#define MUDUO_FILEHELPER_H

#include <hpsocket/common/GlobalDef.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

#define CHECK_IS_OK(expr)				{if(IS_NOT_OK(expr)) return FALSE;}
#define CHECK_ERROR_FD(fd)				{if(IS_INVALID_FD(fd)) return FALSE;}
#define CHECK_ERROR_INVOKE(expr)		{if(!IS_NO_ERROR(expr)) return FALSE;}
#define CHECK_ERROR_CODE(rs)			{if(!IS_NO_ERROR(rs)) { return FALSE;}}
#define CHECK_ERROR(expr, code)			{if(!(expr)) { return FALSE;}}
#define CHECK_EINVAL(expr)				CHECK_ERROR(expr, ERROR_INVALID_PARAMETER)
#define ASSERT_CHECK_ERROR(expr, code)	{ASSERT(expr); CHECK_ERROR(expr, code);}
#define ASSERT_CHECK_EINVAL(expr)		{ASSERT(expr); CHECK_EINVAL(expr);}

#define INVALID_MAP_ADDR	((PBYTE)(MAP_FAILED))

std::string GetCurrentDirectory();
std::string GetModuleFileName(pid_t pid = 0);
BOOL SetCurrentPathToModulePath(pid_t pid = 0);

class FileHelper
{
public:
    BOOL Open(LPCTSTR lpszFilePath, int iFlag, mode_t iMode = 0);
    BOOL Close();
    BOOL Stat(struct stat& st);
    BOOL GetSize(SIZE_T& dwSize);

    SSIZE_T Read(PVOID pBuffer, SIZE_T dwCount)
    {return read(m_fd, pBuffer, dwCount);}
    SSIZE_T Write(PVOID pBuffer, SIZE_T dwCount)
    {return write(m_fd, pBuffer, dwCount);}
    SSIZE_T PRead(PVOID pBuffer, SIZE_T dwCount, SIZE_T dwOffset)
    {return pread(m_fd, pBuffer, dwCount, dwOffset);}
    SSIZE_T PWrite(PVOID pBuffer, SIZE_T dwCount, SIZE_T dwOffset)
    {return pwrite(m_fd, pBuffer, dwCount, dwOffset);}
    SSIZE_T ReadV(const iovec* pVec, int iVecCount)
    {return readv(m_fd, pVec, iVecCount);}
    SSIZE_T WriteV(const iovec* pVec, int iVecCount)
    {return writev(m_fd, pVec, iVecCount);}
    SSIZE_T Seek(SSIZE_T lOffset, int iWhence)
    {return lseek(m_fd, lOffset, iWhence);}

    BOOL IsValid()	{return m_fd != -1;}
    operator FD ()	{return m_fd;}

    BOOL IsExist()	{return IsValid();}

    BOOL IsDirectory();
    BOOL IsFile();

    static BOOL IsExist(LPCTSTR lpszFilePath);
    static BOOL IsDirectory(LPCTSTR lpszFilePath);
    static BOOL IsFile(LPCTSTR lpszFilePath);
    static BOOL IsLink(LPCTSTR lpszFilePath);

public:
    FileHelper(LPCTSTR lpszFilePath = nullptr, int iFlag = O_RDONLY, mode_t iMode = 0)
            : m_fd(INVALID_FD)
    {
        if(lpszFilePath != nullptr)
            Open(lpszFilePath, iFlag, iMode);
    }

    ~FileHelper()
    {
        if(IsValid())
            Close();
    }

private:
    FD m_fd;
};

class FileHelperMapping
{
public:
    BOOL Map(LPCTSTR lpszFilePath, SIZE_T dwSize = 0, SIZE_T dwOffset = 0, int iProtected = PROT_READ, int iFlag = MAP_PRIVATE);
    BOOL Map(FD fd, SIZE_T dwSize = 0, SIZE_T dwOffset = 0, int iProtected = PROT_READ, int iFlag = MAP_PRIVATE);
    BOOL Unmap();
    BOOL MSync(int iFlag = MS_SYNC, SIZE_T dwSize = 0);

    BOOL IsValid	()		{return m_pv != INVALID_MAP_ADDR;}
    SIZE_T Size		()		{return m_dwSize;}
    LPBYTE Ptr		()		{return m_pv;}
    operator LPBYTE	()		{return Ptr();}

public:
    FileHelperMapping()
            : m_pv(INVALID_MAP_ADDR)
            , m_dwSize(0)
    {

    }

    ~FileHelperMapping()
    {
        if(IsValid())
            Unmap();
    }

private:
    PBYTE	m_pv;
    SIZE_T	m_dwSize;
};


#endif //MUDUO_FILEHELPER_H
