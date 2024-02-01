#include <iostream>
#include "/home/riski/FixEngine/QuickfixAppTest/client/test_connect_ori/ouchEngine/header/TEOrderPackage.h"

char szCurDate[8];
void ProcessNewRegularOrder(CTEOrderPackage *pkg)
{
    char szOrderId[32] = {}, szAlt[32] = {}, szCustId[32] = {}, szStockCode[32] = {}, szInputUser[64] = {}, szComplianceId[64] = {}, szOrderSource[16] = {};
    uint8_t command = pkg->GetByte(),
            expire = pkg->GetByte(),
            nationality = pkg->GetByte();
    uint32_t flags = 0,
             price = pkg->GetDWord();
    int64_t volume = pkg->GetInt64();
    uint16_t slen = pkg->GetWord();
    pkg->GetString(slen, szOrderId);
    slen = pkg->GetWord();
    pkg->GetString(slen, szAlt);
    slen = pkg->GetWord();
    pkg->GetString(slen, szCustId);
    slen = pkg->GetWord();
    pkg->GetString(slen, szStockCode);
    slen = pkg->GetWord();
    pkg->GetString(slen, szInputUser);
    slen = pkg->GetWord();
    pkg->GetString(slen, szComplianceId);
    uint32_t orderToken = pkg->GetDWord(),
             orderBookId = 0;
    // orderJournal.WriteLog("OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
    //	szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
    if (strncmp(szOrderId, szCurDate, 6))
    {
        char szRejectText[256] = {};
        // orderJournal.WriteLog("[REJECTED ORDER][PREVIOUS DAY ORDER] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
        //                       szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
        strcpy(szRejectText, "Can not send previous day order, order rejected.");
        CTEOrderPackage *p = new CTEOrderPackage(292, PK_RT_RPL_REJECTED_ORDER);
        if (NULL == p)
            return;
        p->AddString(szOrderId);
        p->AddString(szRejectText);
        outgoing_te_packages.AddtoQueue(p);
        return;
    }
    if (!rgsi.GetStockFlags(szStockCode, flags)) //	Stock Code not found
    {
        // char szRejectText[256] = {};
        // orderJournal.WriteLog("[REJECTED ORDER][STOCK NOT FOUND] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
        //                       szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
        // strcpy(szRejectText, "Stock Code not found in Regular Market.");
        CTEOrderPackage *p = new CTEOrderPackage(292, PK_RT_RPL_REJECTED_ORDER);
        if (NULL == p)
            return;
        p->AddString(szOrderId);
        p->AddString(szRejectText);
        outgoing_te_packages.AddtoQueue(p);
        return;
    }
    char szSecurityCode[64] = {};
    sprintf(szSecurityCode, "%s.%s", szStockCode, BOARD_RG);
    orderBookId = orderBookDir.GetOrderBookId(szSecurityCode);
    if (!order_check.insert(szOrderId))
    {
        orderJournal.WriteLog("[DUPLICATE ORDER] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
                              szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
        return;
    }
    // COuchEnterOrder *o = new COuchEnterOrder(orderToken, szOrderId, szComplianceId, ConvertOrderVerb(command), szOrderSource,
    // 	(0 == nationality) ? DOMICILE_INDONESIA : DOMICILE_ASING, volume / SHARES_LOT, orderBookId, price, ConvertTIF(expire), 0,
    // 	(EXP_FOK_ORDER == expire) ? volume / SHARES_LOT : (EXP_IOC_ORDER == expire) ? 1 : 0, szCustId, szSecurityCode, szInputUser);
    COuchEnterOrder *o = new COuchEnterOrder(orderToken, szOrderId, szComplianceId, ConvertOrderVerb(command), szOrderSource,
                                             (0 == nationality) ? DOMICILE_INDONESIA : DOMICILE_ASING, volume / SHARES_LOT, orderBookId, (0 == price) ? MAX_PRICE : price, ConvertTIF(expire), 0,
                                             (EXP_FOK_ORDER == expire) ? volume / SHARES_LOT : 0, szCustId, szSecurityCode, szInputUser);
    if (NULL == o)
    {
        journal.WriteLog("[ProcessNewRegularOrder] - Memory Exception.\n");
        orderJournal.WriteLog("[MEMORY EXCEPTION] - OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
                              szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
        return;
    }
    if (flags & CStockInfo::SI_WATCH_CALL)
        callOutgoingQ.AddtoQueue(o, pkg->isFirstPriority() ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
    else
    {
        if (flags & CStockInfo::SI_PREOPENING)
            outgoingQ.AddPreoptoQueue(o, pkg->isFirstPriority() ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
        else
            outgoingQ.AddtoQueue(o, pkg->isFirstPriority() ? __OUCH_OUTQ__::PRIOR_HEAD : __OUCH_OUTQ__::PRIOR_TAIL);
    }
    orderJournal.WriteLog("OrderId:%s; Cmd:%d; Stock:%s.%s; OrderBookId:%lu; CustId:%s; Alt:%s; DF:%u; Price:%lu; Volume:%ld; Exp:%d; ComplianceId:%s; Input:%s;\n",
                          szOrderId, command, szStockCode, BOARD_RG, orderBookId, szCustId, szAlt, nationality, price, volume, expire, szComplianceId, szInputUser);
}