#include "scnet/socket/scwebsocketserver_p.h"

#include "HPSocket/HPSocket.h"
#include "scutils/text/scstring.h"
#include "scutils/io/sclogger.h"

class ScWebSocketServerContext : public CHttpServerListener
{
	friend class ScWebSocketServerPrivate;
public:
	ScWebSocketServerContext();

	EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, UINT_PTR soClient) override; // 接收连接回调，WebSocket 握手请求到达时会调用本回调
	EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) override;// 断开连接回调，WebSocket 连接断开时会调用本回调

	EnHttpParseResult OnHeader(IHttpServer* pSender, CONNID dwConnID, LPCSTR lpszName, LPCSTR lpszValue) override;// HTTP 请求头回调，WebSocket 握手请求头也会调用本回调
	EnHttpParseResult OnHeadersComplete(IHttpServer* pSender, CONNID dwConnID) override; // HTTP 请求头解析完成回调，WebSocket 握手请求头解析完成后也会调用本回调
	EnHttpParseResult OnBody(IHttpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength) override; // HTTP 消息体数据回调，WebSocket 握手请求体也会调用本回调
	EnHttpParseResult OnMessageComplete(IHttpServer* pSender, CONNID dwConnID) override; // HTTP 消息解析完成回调，WebSocket 握手完成后也会调用本回调
	EnHttpParseResult OnUpgrade(IHttpServer* pSender, CONNID dwConnID, EnHttpUpgradeType enUpgradeType) override; // 升级协议回调，WebSocket 握手完成后会调用本回调
	EnHttpParseResult OnParseError(IHttpServer* pSender, CONNID dwConnID, int iErrorCode, LPCSTR lpszErrorDesc) override; // HTTP 解析错误回调，WebSocket 握手过程中发生的 HTTP 解析错误也会调用本回调

	EnHandleResult OnWSMessageHeader(IHttpServer* pSender, CONNID dwConnID, BOOL bFinal, BYTE iReserved, BYTE iOperationCode, const BYTE lpszMask[4], ULONGLONG ullBodyLen) override; // 接收消息头回调，注意控制消息也会调用本回调
	EnHandleResult OnWSMessageBody(IHttpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength) override; // 接收消息体数据回调，注意控制消息也会调用本回调
	EnHandleResult OnWSMessageComplete(IHttpServer* pSender, CONNID dwConnID) override;  // 接收完整消息后回调，注意控制消息也会调用本回调

private:
	EnHttpParseResult rejectHandShake(IHttpServer* pSender, CONNID dwConnID, USHORT statusCode, LPCSTR desc, const THeader* headers, int headerCount, LPCSTR body = nullptr);

	bool isHeaderValueEmpty(IHttpServer* pSender, CONNID dwConnID, LPCSTR headerName);
	bool isHeaderContainsToken(LPCSTR headerValue, const ScString& expectedToken);
	bool isHeaderValid(IHttpServer* pSender, CONNID dwConnID, const ScString& key, const ScString& expectedToken);

	std::size_t tryParseCacheSize(LPCSTR value);

	CHttpServerPtr httpServer;
	ScAbstractSocket::SocketState state;
};

struct ScConnectionContext
{
	std::size_t textCacheSize;
	std::size_t binaryCacheSize;
};

ScWebSocketServerPrivate::ScWebSocketServerPrivate(ScWebSocketServer* q, const ScString& serverName, ScWebSocketServer::SslMode mode)
	: q_ptr(q), serverName(serverName), secureMode(mode), m_pContext(new ScWebSocketServerContext())
{
}

ScWebSocketServerPrivate::~ScWebSocketServerPrivate()
{
	SC_SAVE_DELETE(m_pContext);
}

bool ScWebSocketServerPrivate::listen(const ScHostAddress& address, scuint16 port)
{
	if (ScAbstractSocket::ListeningState == m_pContext->state)
	{
		SC_WARN("[%s]Called when already listening!", __FUNCTION__);
		return false;
	}

	if (!m_pContext->httpServer)
	{
		SC_CRITICAL("[%s]HTTP service unavailable: not created!", __FUNCTION__);
		return false;
	}

	if (!m_pContext->httpServer->Start(address.toString().data(), port))
	{
		return false;
	}

	m_pContext->state = ScAbstractSocket::ListeningState;
	return true;
}

void ScWebSocketServerPrivate::close()
{
}

ScWebSocketServerContext::ScWebSocketServerContext()
	: httpServer(this)
	, state(ScAbstractSocket::UnconnectedState)
{
}

EnHandleResult ScWebSocketServerContext::OnAccept(ITcpServer* pSender, CONNID dwConnID, UINT_PTR soClient)
{
	SC_UNUSED(soClient);
	auto* pContext = new ScConnectionContext();
	if (!pSender->SetConnectionExtra(dwConnID, pContext))
	{
		delete pContext;
		return HR_ERROR;
	}
	return HR_OK;
}

EnHandleResult ScWebSocketServerContext::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	PVOID extra = nullptr;
	if (pSender->GetConnectionExtra(dwConnID, &extra) && extra)
	{
		delete static_cast<ScConnectionContext*>(extra);
		pSender->SetConnectionExtra(dwConnID, nullptr);
	}
	return HR_OK;
}

EnHttpParseResult ScWebSocketServerContext::OnHeader(IHttpServer* pSender, CONNID dwConnID, LPCSTR lpszName, LPCSTR lpszValue)
{
	if (!lpszName || !lpszValue)
		return HPR_OK;

	ScString name = ScString(lpszName).toLower();
	std::size_t cacheSize = tryParseCacheSize(lpszValue);
	if ("x-cache-size" == name)
	{
	}
	else if ("x-text-cache-size" == name)
	{

	}
	else if ("x-binary-cache-size" == name)
	{

	}

	return HPR_OK;
}

EnHttpParseResult ScWebSocketServerContext::OnHeadersComplete(IHttpServer* pSender, CONNID dwConnID)
{
	if (HUT_WEB_SOCKET == pSender->GetUpgradeType(dwConnID))
		return HPR_UPGRADE;
	return HPR_OK;
}

EnHttpParseResult ScWebSocketServerContext::OnBody(IHttpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	SC_UNUSED(pSender); SC_UNUSED(dwConnID); SC_UNUSED(pData); SC_UNUSED(iLength);
	return HPR_OK;
}

EnHttpParseResult ScWebSocketServerContext::OnMessageComplete(IHttpServer* pSender, CONNID dwConnID)
{
	if (pSender->GetUpgradeType(dwConnID) == HUT_WEB_SOCKET || pSender->IsUpgrade(dwConnID))
		return HPR_OK;

	static constexpr char body[] = "HPSocket WebSocket server is running.\nUse WebSocket and optional header X-Cache-Size to set per-connection cache size.\n";
	const THeader headers[] =
	{
		{"Content-Type", "text/plain; charset=utf-8"},
		{"Connection", "close"}
	};

	if (!pSender->SendResponse(dwConnID, HSC_OK, "OK", headers, 2, reinterpret_cast<const BYTE*>(body), static_cast<int>(sizeof(body) - 1)))
		return HPR_ERROR;

	pSender->Release(dwConnID);
	return HPR_OK;
}

EnHttpParseResult ScWebSocketServerContext::OnUpgrade(IHttpServer* pSender, CONNID dwConnID, EnHttpUpgradeType enUpgradeType)
{
	if (HUT_WEB_SOCKET != enUpgradeType)
		return HPR_OK;

	const LPCSTR method = pSender->GetMethod(dwConnID);
	if (!method || 0 != ScString("GET").compare(method, strlen(method), Sc::CaseInsensitive))
	{
		const THeader headers[] = { {"Connection", "close"} };
		return rejectHandShake(pSender, dwConnID, HSC_METHOD_NOT_ALLOWED, "Method Not Allowed", headers, 1, "WebSocket handshake requires GET.\n");
	}

	if (isHeaderValueEmpty(pSender, dwConnID, "Host"))
	{
		const THeader headers[] = { {"Connection", "close"} };
		return rejectHandShake(pSender, dwConnID, HSC_BAD_REQUEST, "Bad Request", headers, 1, "Missing Host header.\n");
	}

	if (!isHeaderValid(pSender, dwConnID, "Upgrade", "websocket"))
	{
		const THeader headers[] = { {"Connection", "close"} };
		return rejectHandShake(pSender, dwConnID, HSC_BAD_REQUEST, "Bad Request", headers, 1, "Invalid Connection header.\n");
	}

	if (!isHeaderValid(pSender, dwConnID, "Connection", "Upgrade"))
	{
		const THeader headers[] = { {"Connection", "close"} };
		return rejectHandShake(pSender, dwConnID, HSC_BAD_REQUEST, "Bad Request", headers, 1, "Invalid Connection header.\n");
	}

	LPCSTR versionHeader = nullptr;
	if (!pSender->GetHeader(dwConnID, "Sec-WebSocket-Version", &versionHeader) || !versionHeader
		|| 0 != ScStringView(versionHeader).trimmed().compare(ScString("13")))
	{
		const THeader headers[] =
		{
			{"Connection", "close"},
			{"Sec-WebSocket-Version", "13"}
		};
		return rejectHandShake(pSender, dwConnID, HSC_UPGRADE_REQUIRED, "Upgrade Required", headers, 2, "Unsupported WebSocket version. Required: 13.\n");
	}

	if (isHeaderValueEmpty(pSender, dwConnID, "Sec-WebSocket-Key"))
	{
		const THeader headers[] = { {"Connection", "close"} };
		return rejectHandShake(pSender, dwConnID, HSC_BAD_REQUEST, "Bad Request", headers, 1, "Missing Sec-WebSocket-Key header.\n");
	}

	ScString acceptValue;
	const THeader headers[] =
	{
		{ "Connection",				"Upgrade"},
		{ "Upgrade",				"websocket"},
		{ "Sec-WebSocket-Accept",	acceptValue.data()}
	};

	if (!pSender->SendResponse(dwConnID, HSC_SWITCHING_PROTOCOLS, "Switching Protocols", headers, 3))
		return HPR_ERROR;

	return HPR_OK;
}

EnHttpParseResult ScWebSocketServerContext::OnParseError(IHttpServer* pSender, CONNID dwConnID, int iErrorCode, LPCSTR lpszErrorDesc)
{
	return EnHttpParseResult();
}

EnHandleResult ScWebSocketServerContext::OnWSMessageHeader(IHttpServer* pSender, CONNID dwConnID, BOOL bFinal, BYTE iReserved, BYTE iOperationCode, const BYTE lpszMask[4], ULONGLONG ullBodyLen)
{
	return EnHandleResult();
}

EnHandleResult ScWebSocketServerContext::OnWSMessageBody(IHttpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	return EnHandleResult();
}

EnHandleResult ScWebSocketServerContext::OnWSMessageComplete(IHttpServer* pSender, CONNID dwConnID)
{
	return EnHandleResult();
}

EnHttpParseResult ScWebSocketServerContext::rejectHandShake(IHttpServer* pSender, CONNID dwConnID, USHORT statusCode, LPCSTR desc, const THeader* headers, int headerCount, LPCSTR body)
{
	const BYTE* bodyData = body == nullptr ? nullptr : reinterpret_cast<const BYTE*>(body);
	const int bodyLength = body == nullptr ? 0 : static_cast<int>(std::strlen(body));
	pSender->SendResponse(dwConnID, statusCode, desc, headers, headerCount, bodyData, bodyLength);
	pSender->Release(dwConnID);
	return HPR_ERROR;
}

bool ScWebSocketServerContext::isHeaderValueEmpty(IHttpServer* pSender, CONNID dwConnID, LPCSTR headerName)
{
	LPCSTR headerValue = nullptr;
	return pSender->GetHeader(dwConnID, headerName, &headerValue)
		&& headerValue != nullptr
		&& !ScString(headerValue).trimmed().isEmpty();
}

bool ScWebSocketServerContext::isHeaderContainsToken(LPCSTR headerValue, const ScString& expectedToken)
{
	if (!headerValue)
		return false;

	ScString token;
	ScString value(headerValue);
	std::size_t start = 0, end = 0;
	while (start <= value.size())
	{
		end = value.indexOf(',', start);
		token = value.mid(start, ScString::npos == end ? ScString::npos : end - start).trimmed();
		if (0 == token.compare(expectedToken, Sc::CaseInsensitive))
			return true;

		if (ScString::npos == end)
			break;
		start = end + 1;
	}

	return false;
}

inline bool ScWebSocketServerContext::isHeaderValid(IHttpServer* pSender, CONNID dwConnID, const ScString& key, const ScString& expectedToken)
{
	LPCSTR headerValue;
	if (!pSender->GetHeader(dwConnID, key.data(), &headerValue) || !isHeaderContainsToken(headerValue, expectedToken))
		return false;
	return true;
}

std::size_t ScWebSocketServerContext::tryParseCacheSize(LPCSTR value)
{
	if (!value)
		return -1;

	char* end = nullptr;
	const unsigned long long parsed = std::strtoull(value, &end, 10);
	if (end == value || (end && *end != '\0') || parsed == 0)
		return -1;
	return static_cast<std::size_t>(parsed);
}
