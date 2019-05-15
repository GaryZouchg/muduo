//
// Created by test2 on 19-5-15.
//

#include "FileHelper.h"
#include <sys/stat.h>


std::string GetCurrentDirectory()
{
    char szPath[MAX_PATH];

    if(getcwd(szPath, sizeof(szPath) - 1) == nullptr)
        szPath[0] = 0;

    return szPath;
}

std::string GetModuleFileName(pid_t pid)
{
    if(pid == 0)
        pid = getpid();

    char szLink[MAX_PATH];
    char szPath[MAX_PATH];

    sprintf(szLink, "/proc/%d/exe", pid);

    SSIZE_T rs = readlink(szLink, szPath, sizeof(szPath) - 1);

    if(rs < 0) rs = 0;
    szPath[rs]	  = 0;

    return szPath;
}

BOOL SetCurrentPathToModulePath(pid_t pid)
{
    std::string strPath = GetModuleFileName(pid);

    if(strPath.empty())
        return FALSE;

    size_t pos = strPath.rfind('/');
    if(pos == std::string::npos)
        return FALSE;


    return chdir(strPath.substr(0,pos+1).c_str());
}

BOOL FileHelper::Open(LPCTSTR lpszFilePath, int iFlag, mode_t iMode)
{
    if (IsValid())
        return FALSE;

    m_fd = open(lpszFilePath, iFlag, iMode);

    return m_fd != -1;
}

BOOL FileHelper::Close()
{
    if (!IsValid())
        return FALSE;

    if(close(m_fd) ==0)
    {
        m_fd = INVALID_FD;
        return TRUE;
    }

    return FALSE;
}

BOOL FileHelper::Stat(struct stat& st)
{
    if (fstat(m_fd, &st) < 0)
        return FALSE;
    return TRUE;
}

BOOL FileHelper::GetSize(SIZE_T& dwSize)
{
    struct stat st;
    if (!Stat(st))
        return FALSE;

    dwSize = st.st_size;

    return TRUE;
}

BOOL FileHelper::IsDirectory()
{
    struct stat st;
    if (!Stat(st))
        return FALSE;

    return S_ISDIR(st.st_mode);
}

BOOL FileHelper::IsFile()
{
    struct stat st;
    if (!Stat(st))
        return FALSE;

    return S_ISREG(st.st_mode);
}

BOOL FileHelper::IsExist(LPCTSTR lpszFilePath)
{
    return access(lpszFilePath, F_OK);
}

BOOL FileHelper::IsDirectory(LPCTSTR lpszFilePath)
{
    struct stat st;
    if (stat(lpszFilePath, &st) < 0)
        return false;

    return S_ISDIR(st.st_mode);
}

BOOL FileHelper::IsFile(LPCTSTR lpszFilePath)
{
    struct stat st;
    if (stat(lpszFilePath, &st) < 0)
        return FALSE;

    return S_ISREG(st.st_mode);
}

BOOL FileHelper::IsLink(LPCTSTR lpszFilePath)
{
    struct stat st;
    if (lstat(lpszFilePath, &st) < 0)
        return FALSE;

    return S_ISLNK(st.st_mode);
}

BOOL FileHelperMapping::Map(LPCTSTR lpszFilePath, SIZE_T dwSize, SIZE_T dwOffset, int iProtected, int iFlag)
{
    CHECK_ERROR(!IsValid(), ERROR_INVALID_STATE);

    FD fd = INVALID_FD;

    if(lpszFilePath != nullptr)
    {
        int iFileFlag = O_RDONLY;

        if(iProtected & PROT_WRITE)
        {
            if(iProtected & PROT_READ)
                iFileFlag = O_RDWR;
            else
                iFileFlag = O_WRONLY;
        }

        fd = open(lpszFilePath, iFileFlag);
        if (fd == -1)
            return FALSE;
    }

    BOOL isOK = Map(fd, dwSize, dwOffset, iProtected, iFlag);

    if (fd != -1)
        close(fd);

    return isOK;
}

BOOL FileHelperMapping::Map(FD fd, SIZE_T dwSize, SIZE_T dwOffset, int iProtected, int iFlag)
{
    CHECK_ERROR(!IsValid(), ERROR_INVALID_STATE);

    if(fd == -1)
    {
        CHECK_EINVAL((iFlag & MAP_ANONYMOUS) && (dwSize > 0));
    }
    else
    {
        CHECK_EINVAL((iFlag & MAP_ANONYMOUS) == 0);

        struct stat st;

        if (fstat(fd, &st) < 0)
            return  FALSE;

        CHECK_ERROR(S_ISREG(st.st_mode), ERROR_BAD_FILE_TYPE);

        if(dwSize == 0)
            dwSize = st.st_size;
    }

    m_pv = (PBYTE)mmap(nullptr, dwSize, iProtected, iFlag, fd, dwOffset);

    if(IsValid())
    {
        m_dwSize = dwSize;
        return TRUE;
    }

    return FALSE;
}

BOOL FileHelperMapping::Unmap()
{
    CHECK_ERROR(IsValid(), ERROR_INVALID_STATE);

    if(munmap(m_pv, m_dwSize) == 0)
    {
        m_pv	 = INVALID_MAP_ADDR;
        m_dwSize = 0;

        return TRUE;
    }

    return FALSE;
}

BOOL FileHelperMapping::MSync(int iFlag, SIZE_T dwSize)
{
    CHECK_ERROR(IsValid(), ERROR_INVALID_STATE);

    if(dwSize == 0) dwSize = m_dwSize;

    return (msync(m_pv, dwSize, iFlag) == 0);
}
