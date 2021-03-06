/*
 * Copyright (c) 2014, AirBitz, Inc.
 * All rights reserved.
 *
 * See the LICENSE file for more information.
 */

#include "LoginShim.hpp"
#include "../abcd/login/Lobby.hpp"
#include "../abcd/login/Login.hpp"
#include "../abcd/login/LoginDir.hpp"
#include "../abcd/login/LoginPassword.hpp"
#include "../abcd/login/LoginPin.hpp"
#include "../abcd/login/LoginRecovery.hpp"
#include "../abcd/login/LoginServer.hpp"
#include <mutex>

namespace abcd {

// This mutex protects the shared_ptr caches themselves.
// Using a reference count ensures that any objects still in use
// on another thread will not be destroyed during a cache update.
// The mutex only needs to be locked when updating the cache,
// not when using the objects inside.
// The cached objects must provide their own thread safety.
std::mutex gLoginMutex;
std::shared_ptr<Lobby> gLobbyCache;
std::shared_ptr<Login> gLoginCache;

/**
 * Clears the cached login.
 * The caller should already be holding the login mutex.
 */
static void
cacheClear()
{
    gLobbyCache.reset();
    gLoginCache.reset();
}

void
cacheLogout()
{
   std::lock_guard<std::mutex> lock(gLoginMutex);
   cacheClear();
}

Status
cacheLobby(std::shared_ptr<Lobby> &result, const char *szUserName)
{
    std::lock_guard<std::mutex> lock(gLoginMutex);

    if (!szUserName)
        return ABC_ERROR(ABC_CC_NULLPtr, "No user name");

    // Clear the cache if the username has changed:
    if (gLobbyCache)
    {
        std::string fixed;
        ABC_CHECK(Lobby::fixUsername(fixed, szUserName));
        if (gLobbyCache->username() != fixed)
            cacheClear();
    }

    // Load the new lobby, if necessary:
    if (!gLobbyCache)
    {
        std::unique_ptr<Lobby> lobby(new Lobby());
        ABC_CHECK(lobby->init(szUserName));
        gLobbyCache.reset(lobby.release());
    }

    result = gLobbyCache;
    return Status();
}

Status
cacheLoginNew(std::shared_ptr<Login> &result,
    const char *szUserName, const char *szPassword)
{
    std::shared_ptr<Lobby> lobby;
    ABC_CHECK(cacheLobby(lobby, szUserName));

    // Log the user in, if necessary:
    std::lock_guard<std::mutex> lock(gLoginMutex);
    if (!gLoginCache)
    {
        ABC_CHECK_OLD(ABC_LoginCreate(gLoginCache, lobby, szPassword, &error));
    }

    result = gLoginCache;
    return Status();
}

Status
cacheLoginPassword(std::shared_ptr<Login> &result,
    const char *szUserName, const char *szPassword)
{
    std::shared_ptr<Lobby> lobby;
    ABC_CHECK(cacheLobby(lobby, szUserName));

    // Log the user in, if necessary:
    std::lock_guard<std::mutex> lock(gLoginMutex);
    if (!gLoginCache)
    {
        ABC_CHECK_OLD(ABC_LoginPassword(gLoginCache, lobby, szPassword, &error));
    }

    result = gLoginCache;
    return Status();
}

Status
cacheLoginRecovery(std::shared_ptr<Login> &result,
    const char *szUserName, const char *szRecoveryAnswers)
{
    std::shared_ptr<Lobby> lobby;
    ABC_CHECK(cacheLobby(lobby, szUserName));

    // Log the user in, if necessary:
    std::lock_guard<std::mutex> lock(gLoginMutex);
    if (!gLoginCache)
    {
        ABC_CHECK_OLD(ABC_LoginRecovery(gLoginCache, lobby, szRecoveryAnswers, &error));
    }

    result = gLoginCache;
    return Status();
}

Status
cacheLoginPin(std::shared_ptr<Login> &result,
    const char *szUserName, const char *szPin)
{
    std::shared_ptr<Lobby> lobby;
    ABC_CHECK(cacheLobby(lobby, szUserName));

    // Log the user in, if necessary:
    std::lock_guard<std::mutex> lock(gLoginMutex);
    if (!gLoginCache)
    {
        ABC_CHECK_OLD(ABC_LoginPin(gLoginCache, lobby, szPin, &error));
    }

    result = gLoginCache;
    return Status();
}

Status
cacheLogin(std::shared_ptr<Login> &result, const char *szUserName)
{
    std::lock_guard<std::mutex> lock(gLoginMutex);

    // Get the real username:
    if (!szUserName)
        return ABC_ERROR(ABC_CC_NULLPtr, "No user name");
    std::string fixed;
    ABC_CHECK(Lobby::fixUsername(fixed, szUserName));

    // Ensure the user is still logged in:
    if (!gLoginCache || !gLobbyCache || gLobbyCache->username() != fixed)
        return ABC_ERROR(ABC_CC_AccountDoesNotExist, "Not logged in");

    result = gLoginCache;
    return Status();
}

/**
 * Obtains the information needed to access the server for a given account.
 *
 * @param szUserName UserName for the account to access
 * @param szPassword Password for the account to access
 * @param pL1        A buffer to receive L1. The caller must free this.
 * @param pLP1       A buffer to receive LP1. The caller must free this.
 * @param pError     A pointer to the location to store the error if there is one
 */

tABC_CC ABC_LoginShimGetServerKeys(const char *szUserName,
                                   tABC_U08Buf *pL1,
                                   tABC_U08Buf *pLP1,
                                   tABC_Error *pError)
{
    tABC_CC cc = ABC_CC_Ok;
    std::shared_ptr<Login> login;

    ABC_CHECK_NEW(cacheLogin(login, szUserName), pError);
    ABC_CHECK_RET(ABC_LoginGetServerKeys(*login, pL1, pLP1, pError));

exit:
    return cc;
}

} // namespace abcd
