#ifndef DWEBSERVICEAPI_H
#define DWEBSERVICEAPI_H

/*****************************************************************************
**
**  Definition of the DWebServiceApi class
**
**  Creation date:  2023/10/28
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2023 Piql AS.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 3 of the License, or
**  any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include "ddatatypes.h"
#include "dcontext.h"

//  QT INCLUDES
//
#include <QtHttpServer/QHttpServer>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore/QJsonArray>


//============================================================================
// CLASS: DWebServiceApi

class DWebServiceApi
{
public:
    typedef QString DAipId;

public:
    explicit DWebServiceApi(const DContext& context)
        : m_Context(context) 
    {
    }

    QHttpServerResponse getItems() const
    {
        return QHttpServerResponse(DDataTypes::toJson(m_Context.m_Imports));
    }

    QHttpServerResponse getItem(const DAipId& itemId) const
    {
        const auto item = GetImport(m_Context.m_Imports, itemId);
        return item != nullptr ? QHttpServerResponse(DDataTypes::toJson(item))
            : QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
    }

    QHttpServerResponse getNodes(const DAipId& itemId) const
    {
        const auto item = GetImport(m_Context.m_Imports, itemId);
        return item != nullptr ? QHttpServerResponse(DDataTypes::toJson(item->root()))
            : QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
    }

    QHttpServerResponse getNode(const DAipId& itemId, qint64 nodeId) const
    {
        const auto item = GetImport(m_Context.m_Imports, itemId);
        /*
        const auto item = m_Context.m_Imports.at(itemId);
        return item != nullptr ? QHttpServerResponse(DDataTypes::toJson())
            : QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
            */
        return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
    }

    QHttpServerResponse load(const DAipId& itemId) const
    {
        const auto import = GetImport(m_Context.m_Imports, itemId);

        if (import == nullptr)
        {
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
        }

        if (import->state() == DImport::IMPORT_STATE_REPORT_IMPORTED) 
        {
            const DImportFormat* format = m_Context.m_ImportFormats.find(import->formatName());
            import->load(format);
            return QHttpServerResponse( QJsonObject{ { "status", "loading" } });
        }

        if (import->state() == DImport::IMPORT_STATE_DONE)
        {
            return QHttpServerResponse(QJsonObject{ { "error", "already loaded" } });
        } 
        else if (import->state() == DImport::IMPORT_STATE_IMPORTING)
        {
            return QHttpServerResponse(QJsonObject{ { "status", "importing" } });
        }
        else if (import->state() == DImport::IMPORT_STATE_INDEXING)
        {
            return QHttpServerResponse(QJsonObject{ { "status", "indexing" } });
        }
        else if (import->state() == DImport::IMPORT_STATE_CANCELING)
        {
            return QHttpServerResponse(QJsonObject{ { "status", "canceling" } });
        }

        return QHttpServerResponse(QJsonObject{ { "error", "illegal state" } });
    }

    /*
    QFuture<QHttpServerResponse> getPaginatedList(const QHttpServerRequest& request) const
    {
        using PaginatorType = Paginator<IdMap<T>>;
        std::optional<qsizetype> maybePage;
        std::optional<qsizetype> maybePerPage;
        std::optional<qint64> maybeDelay;
        if (request.query().hasQueryItem("page"))
            maybePage = request.query().queryItemValue("page").toLongLong();
        if (request.query().hasQueryItem("per_page"))
            maybePerPage = request.query().queryItemValue("per_page").toLongLong();
        if (request.query().hasQueryItem("delay"))
            maybeDelay = request.query().queryItemValue("delay").toLongLong();

        if ((maybePage && *maybePage < 1) || (maybePerPage && *maybePerPage < 1)) {
            return QtConcurrent::run([]() {
                return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
                });
        }

        PaginatorType paginator(data, maybePage ? *maybePage : PaginatorType::defaultPage,
            maybePerPage ? *maybePerPage : PaginatorType::defaultPageSize);

        return QtConcurrent::run([paginator = std::move(paginator), maybeDelay]() {
            if (maybeDelay)
                QThread::sleep(*maybeDelay);
            return paginator.isValid()
                ? QHttpServerResponse(paginator.toJson())
                : QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
        });
    }
    */

    /*
    QHttpServerResponse postItem(const QHttpServerRequest& request)
    {
        const std::optional<QJsonObject> json = byteArrayToJsonObject(request.body());
        if (!json)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        const std::optional<T> item = factory->fromJson(*json);
        if (!item)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
        if (data.contains(item->id))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::AlreadyReported);

        const auto entry = data.insert(item->id, *item);
        return QHttpServerResponse(entry->toJson(), QHttpServerResponder::StatusCode::Created);
    }

    QHttpServerResponse updateItem(qint64 itemId, const QHttpServerRequest& request)
    {
        const std::optional<QJsonObject> json = byteArrayToJsonObject(request.body());
        if (!json)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        auto item = data.find(itemId);
        if (item == data.end())
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
        if (!item->update(*json))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        return QHttpServerResponse(item->toJson());
    }

    QHttpServerResponse updateItemFields(qint64 itemId, const QHttpServerRequest& request)
    {
        const std::optional<QJsonObject> json = byteArrayToJsonObject(request.body());
        if (!json)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        auto item = data.find(itemId);
        if (item == data.end())
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
        item->updateFields(*json);

        return QHttpServerResponse(item->toJson());
    }

    QHttpServerResponse deleteItem(qint64 itemId)
    {
        if (!data.remove(itemId))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
        return QHttpServerResponse(QHttpServerResponder::StatusCode::Ok);
    }
    */
private:
    const DContext& m_Context;
};

/*
class SessionApi
{
public:
    explicit SessionApi(const IdMap<SessionEntry>& sessions,
        std::unique_ptr<FromJsonFactory<SessionEntry>> factory)
        : sessions(sessions), factory(std::move(factory))
    {
    }

    QHttpServerResponse registerSession(const QHttpServerRequest& request)
    {
        const auto json = byteArrayToJsonObject(request.body());
        if (!json)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
        const auto item = factory->fromJson(*json);
        if (!item)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        const auto session = sessions.insert(item->id, *item);
        session->startSession();
        return QHttpServerResponse(session->toJson());
    }

    QHttpServerResponse login(const QHttpServerRequest& request)
    {
        const auto json = byteArrayToJsonObject(request.body());

        if (!json || !json->contains("email") || !json->contains("password"))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        auto maybeSession = std::find_if(
            sessions.begin(), sessions.end(),
            [email = json->value("email").toString(),
            password = json->value("password").toString()](const auto& it) {
            return it.password == password && it.email == email;
        });
        if (maybeSession == sessions.end()) {
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
        }
        maybeSession->startSession();
        return QHttpServerResponse(maybeSession->toJson());
    }

    QHttpServerResponse logout(const QHttpServerRequest& request)
    {
        const auto maybeToken = getTokenFromRequest(request);
        if (!maybeToken)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        auto maybeSession = std::find(sessions.begin(), sessions.end(), *maybeToken);
        if (maybeSession != sessions.end())
            maybeSession->endSession();
        return QHttpServerResponse(QHttpServerResponder::StatusCode::Ok);
    }

    bool authorize(const QHttpServerRequest& request) const
    {
        const auto maybeToken = getTokenFromRequest(request);
        if (maybeToken) {
            const auto maybeSession = std::find(sessions.begin(), sessions.end(), *maybeToken);
            return maybeSession != sessions.end() && *maybeSession == *maybeToken;
        }
        return false;
    }

private:
    IdMap<SessionEntry> sessions;
    std::unique_ptr<FromJsonFactory<SessionEntry>> factory;
};
*/
#endif 
