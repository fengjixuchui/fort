#ifndef CONFRULEMANAGER_H
#define CONFRULEMANAGER_H

#include <QObject>

#include <sqlite/sqlitetypes.h>

#include <util/classhelpers.h>
#include <util/ioc/iocservice.h>

class ConfManager;
class Rule;

class ConfRuleManager : public QObject, public IocService
{
    Q_OBJECT

public:
    explicit ConfRuleManager(QObject *parent = nullptr);
    CLASS_DELETE_COPY_MOVE(ConfRuleManager)

    ConfManager *confManager() const;
    SqliteDb *sqliteDb() const;

    void setUp() override;

    void loadRuleSet(Rule &rule, QStringList &ruleSetNames);
    void saveRuleSet(Rule &rule);

    bool checkRuleSetLoop(int ruleId, int subRuleId);

    virtual bool addOrUpdateRule(Rule &rule);
    virtual bool deleteRule(int ruleId);
    virtual bool updateRuleName(int ruleId, const QString &ruleName);
    virtual bool updateRuleEnabled(int ruleId, bool enabled);

    void updateDriverRules(quint32 rulesMask, quint32 enabledMask, quint32 dataSize,
            const QList<QByteArray> &rulesData);

signals:
    void ruleAdded();
    void ruleRemoved(int ruleId);
    void ruleUpdated();

private:
    bool updateDriverRuleFlag(int ruleId, bool enabled);

    int getFreeRuleId(bool &ok);
    void putFreeRuleId(int ruleId);

    bool beginTransaction();
    void commitTransaction(bool &ok);

private:
    ConfManager *m_confManager = nullptr;
};

#endif // CONFRULEMANAGER_H
