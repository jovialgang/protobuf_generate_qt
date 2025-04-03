#include "test_object.h"

#include <gui/object_model/object_model.h>
#include <gui/object_model/object_model_qml.h>
#include <QCoreApplication>
#include <QPointer>
#include <QSignalSpy>
#include <array>
#include <gtest/gtest.h>
#include <log.h>
#include <memory>
#include <type_traits>

using namespace om;

template <class T>
class ModelFixture : public ::testing::Test
{
public:
    void SetUp() override
    {
        int   args = 1;
        char* argv = "";
        om::RegisterQmlTypes();
        app_          = std::make_unique<QCoreApplication>(args, &argv);
        dummy_parent_ = std::make_unique<QObject>();
        model_        = std::make_unique<T>();
        for (int i = 0; i < objects_.size(); ++i)
        {
            objects_[i] = new TestObject(i, QString::number(i), dummy_parent_.get());
            objects_[i]->setObjectName(QString("objectName%1").arg(i));
        }
    }

    void TearDown() override
    {
        objects_.fill(nullptr);
        model_.reset();
        dummy_parent_.reset();
        app_.reset();
    }

protected:
    // COMPONENT_LOGGER("test");
    std::unique_ptr<QCoreApplication> app_;
    std::unique_ptr<T>                model_;
    std::unique_ptr<QObject>          dummy_parent_ = nullptr;
    std::array<TestObject*, 3>        objects_;
};

using ObjectRefModelF = ModelFixture<ObjectRefModel>;
using ObjectModelF    = ModelFixture<ObjectModel>;

TEST_F(ObjectRefModelF, SetItems)
{
    QSignalSpy model_reset_signal(model_.get(), &ObjectRefModel::modelReset);
    model_->SetItems({ objects_[0], objects_[1] });
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[0], objects_[1] }));
    model_->SetItems({ objects_[2] });
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[2] }));
    EXPECT_EQ(model_reset_signal.count(), 2);
}

TEST_F(ObjectRefModelF, InitializationAndRoleNames)
{
    QSignalSpy initialized_signal(model_.get(), &ObjectRefModel::initialized);
    EXPECT_EQ(model_->IsInitialized(), false);
    model_->SetItems({ objects_[0] });
    EXPECT_EQ(model_->IsInitialized(), true);
    EXPECT_EQ(initialized_signal.count(), 1);
    EXPECT_EQ(model_->roleNames().values(), QByteArrayList({ AbstractObjectModel::kItemRoleName }));
}

TEST_F(ObjectRefModelF, GetData)
{
    model_->SetItems({ objects_.begin(), objects_.end() });

    EXPECT_EQ(model_->data(model_->index(0, 0), AbstractObjectModel::kItemRole).value<TestObject*>(), objects_[0]);
    EXPECT_EQ(model_->data(model_->index(0, 0), AbstractObjectModel::kItemRoleName).value<TestObject*>(), objects_[0]);
    EXPECT_EQ(model_->data(model_->index(1, 0), model_->roleIds().value("id")).toInt(), 1);
    EXPECT_EQ(model_->data(model_->index(1, 0), "id").toInt(), 1);
    EXPECT_EQ(model_->GetData(2, model_->roleIds().value("coord")).value<CoordObject*>(), nullptr);
    EXPECT_EQ(model_->GetData(2, "coord").value<CoordObject*>(), nullptr);
    EXPECT_EQ(model_->GetData(2, model_->roleIds().value("coord.type")), QVariant());
    EXPECT_EQ(model_->GetData(2, "coord.type.type"), QVariant());

    objects_[2]->SetCoord(new CoordObject(42, 777, objects_[2]));
    objects_[2]->GetCoord()->SetType(new CoordTypeObject(objects_[2]));
    objects_[2]->GetCoord()->GetType()->SetType(CoordTypeObject::Type::Type3);
    EXPECT_EQ(model_->GetData(2, model_->roleIds().value("coord.x")).toDouble(), 42);
    EXPECT_EQ(model_->GetData(2, "coord.y").toFloat(), 777);
    EXPECT_EQ(model_->GetData(2, "coord.type.type").value<CoordTypeObject::Type>(), CoordTypeObject::Type::Type3);
    EXPECT_EQ(model_->GetData(0, "coord.type.type"), QVariant());

    EXPECT_EQ(model_->GetData(2, "coord.type.changed").value<CoordTypeObject*>(), objects_[2]->GetCoord()->GetType());
    EXPECT_EQ(model_->GetData(0, "changed").value<TestObject*>(), objects_[0]);
    EXPECT_EQ(model_->GetData(0, "coord.type.changed"), QVariant());
}

TEST_F(ObjectRefModelF, DataRoles)
{
    model_->SetItems({ objects_.begin(), objects_.end() });
    EXPECT_EQ(model_->roleNames().size(), 1);
    EXPECT_EQ(model_->roleNames().value(ObjectModel::kItemRole), ObjectModel::kItemRoleName);

    model_->SetDataRoles("coord.*");
    EXPECT_EQ(model_->roleNames().size(), 5);
    EXPECT_NE(model_->roleNames().key("coordTypeType", -1), -1);
    model_->SetDataRoles("coord.*/aon");
    EXPECT_EQ(model_->roleNames().size(), 7);
}

TEST_F(ObjectRefModelF, ItemDestruction)
{
    model_->SetItems({ objects_.begin(), objects_.end() });
    delete objects_[1];
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[0], objects_[2] }));
}

TEST_F(ObjectModelF, ObjectModelParentBehavior)
{
    model_->SetItems({ objects_[0], nullptr, objects_[2] });
    QPointer<TestObject> created_object = static_cast<TestObject*>(model_->operator[](1));
    ASSERT_NE(created_object, nullptr);
    EXPECT_EQ(created_object->parent(), model_.get());
    EXPECT_EQ(objects_[0]->parent(), model_.get());
    EXPECT_EQ(objects_[2]->parent(), model_.get());
    model_->Remove(1);
    EXPECT_EQ(created_object, nullptr);
}

TEST_F(ObjectRefModelF, Append)
{
    QSignalSpy rows_inserted_signal(model_.get(), &ObjectRefModel::rowsInserted);
    model_->Append(objects_[0]);
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[0] }));
    ASSERT_EQ(rows_inserted_signal.count(), 1);
    auto signal_params = rows_inserted_signal.takeFirst();
    EXPECT_EQ(signal_params[1], 0);  // first
    EXPECT_EQ(signal_params[2], 0);  // last
}

TEST_F(ObjectRefModelF, Insert)
{
    QSignalSpy rows_inserted_signal(model_.get(), &ObjectRefModel::rowsInserted);
    model_->AppendVector({ objects_[0], objects_[2] });  // так же протестируем Append(const QVector<QObject*>&)
    model_->Insert(1, objects_[1]);
    ASSERT_EQ(rows_inserted_signal.count(), 2);
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[0], objects_[1], objects_[2] }));
    {  // первый вызов после Append
        auto signal_params = rows_inserted_signal.takeFirst();
        EXPECT_EQ(signal_params[1], 0);  // first
        EXPECT_EQ(signal_params[2], 1);  // last
    }
    {  // второй вызов после Insert
        auto signal_params = rows_inserted_signal.takeFirst();
        EXPECT_EQ(signal_params[1], 1);  // first
        EXPECT_EQ(signal_params[2], 1);  // last
    }
}

TEST_F(ObjectRefModelF, OutOfBoundsInsert)
{
    QSignalSpy rows_inserted_signal(model_.get(), &ObjectRefModel::rowsInserted);
    model_->Insert(1, objects_[0]);
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>());
    EXPECT_EQ(rows_inserted_signal.count(), 0);  // сигнал не должен был вызваться
}

TEST_F(ObjectRefModelF, Set)
{
    QSignalSpy data_changed_signal(model_.get(), &ObjectRefModel::dataChanged);
    model_->AppendVector({ objects_[0], objects_[2] });
    model_->Set(1, objects_[1]);
    EXPECT_EQ(model_->At(1), objects_[1]);
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[0], objects_[1] }));

    ASSERT_EQ(data_changed_signal.count(), 1);
    {
        auto signal_params = data_changed_signal.takeFirst();
        EXPECT_EQ(signal_params[0].toModelIndex().row(), 1);                // topLeft
        EXPECT_EQ(signal_params[1].toModelIndex().row(), 1);                // bottomRight
        EXPECT_EQ(signal_params[2].value<QVector<int>>(), QVector<int>());  // roles
    }
}

TEST_F(ObjectRefModelF, OutOfBoundsSet)
{
    QSignalSpy data_changed_signal(model_.get(), &ObjectRefModel::dataChanged);
    model_->AppendVector({ objects_[0], objects_[1] });
    model_->Set(2, objects_[2]);
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[0], objects_[1] }));
    EXPECT_EQ(data_changed_signal.count(), 0);  // сигнал не должен был вызваться
}

TEST_F(ObjectRefModelF, AddingObjectsOfTheWrongType)
{
    model_->Append(objects_[0]);  // инициализируем модель типом
    QSignalSpy rows_inserted_signal(model_.get(), &ObjectRefModel::rowsInserted);
    QSignalSpy data_changed_signal(model_.get(), &ObjectRefModel::dataChanged);
    QSignalSpy model_reset_signal(model_.get(), &ObjectRefModel::modelReset);

    auto super_object = new TestSuperObject(dummy_parent_.get());

    EXPECT_ANY_THROW({ model_->Append(super_object); });
    EXPECT_ANY_THROW({ model_->Append({ super_object }); });
    EXPECT_ANY_THROW({ model_->Insert(0, super_object); });
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[0] }));

    EXPECT_ANY_THROW({ model_->Set(0, super_object); });
    EXPECT_ANY_THROW({ model_->SetItems({ super_object }); });

    // сигналы не должны были прийти
    EXPECT_EQ(rows_inserted_signal.count(), 0);
    EXPECT_EQ(data_changed_signal.count(), 0);
    EXPECT_EQ(model_reset_signal.count(), 0);
}

TEST_F(ObjectRefModelF, AddingDuplicateObjects)
{
    model_->SetItems({ objects_[0], objects_[1] });
    QSignalSpy rows_inserted_signal(model_.get(), &ObjectRefModel::rowsInserted);
    QSignalSpy data_changed_signal(model_.get(), &ObjectRefModel::dataChanged);
    QSignalSpy model_reset_signal(model_.get(), &ObjectRefModel::modelReset);

    EXPECT_ANY_THROW({ model_->Append(objects_[1]); });
    EXPECT_ANY_THROW({ model_->AppendVector({ objects_[2], objects_[1] }); });
    EXPECT_ANY_THROW({ model_->Insert(0, objects_[1]); });
    EXPECT_ANY_THROW({ model_->Set(0, objects_[1]); });

    // сигналы не должны были прийти
    EXPECT_EQ(rows_inserted_signal.count(), 0);
    EXPECT_EQ(data_changed_signal.count(), 0);
    EXPECT_EQ(model_reset_signal.count(), 0);
}

TEST_F(ObjectRefModelF, Remove)
{
    QSignalSpy rows_removed_signal(model_.get(), &ObjectRefModel::rowsRemoved);
    model_->AppendVector({ objects_.begin(), objects_.end() });

    model_->Remove(1);
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[0], objects_[2] }));

    model_->Remove(objects_[0]);
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[2] }));

    model_->Remove("objectName2");
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>());

    EXPECT_EQ(rows_removed_signal.count(), 3);
}

TEST_F(ObjectRefModelF, Clear)
{
    QSignalSpy model_reset_signal(model_.get(), &ObjectRefModel::modelReset);
    model_->AppendVector({ objects_.begin(), objects_.end() });

    model_->Clear();
    EXPECT_EQ(model_->GetItems(), QVector<QObject*>());

    EXPECT_EQ(model_reset_signal.count(), 1);
}

TEST_F(ObjectRefModelF, Take)
{
    QSignalSpy rows_removed_signal(model_.get(), &ObjectRefModel::rowsRemoved);

    model_->AppendVector({ objects_.begin(), objects_.end() });
    {
        auto taken = model_->Take(1);
        EXPECT_EQ(taken, objects_[1]);
        EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[0], objects_[2] }));
    }
    {
        auto taken = model_->TakeFirst();
        EXPECT_EQ(taken, objects_[0]);
        EXPECT_EQ(model_->GetItems(), QVector<QObject*>({ objects_[2] }));
    }
    {
        auto taken = model_->TakeLast();
        EXPECT_EQ(taken, objects_[2]);
        EXPECT_EQ(model_->GetItems(), QVector<QObject*>());
    }

    EXPECT_EQ(rows_removed_signal.count(), 3);
}

TEST_F(ObjectRefModelF, IndexOf)
{
    model_->AppendVector({ objects_.begin(), objects_.end() });
    objects_[1]->SetCoord(new CoordObject(42, 777, objects_[1]));
    EXPECT_EQ(model_->IndexOf(objects_[1]), 1);
    EXPECT_EQ(model_->IndexOf("objectName0"), 0);
    EXPECT_EQ(model_->IndexOf("name", "2"), 2);
    EXPECT_EQ(model_->IndexOf({ { "coord.x", 42 }, { "coord.y", 777 } }), 1);

    EXPECT_EQ(model_->IndexOf(objects_[1]->GetCoord()), -1);
    EXPECT_EQ(model_->IndexOf("someObject"), -1);
    EXPECT_EQ(model_->IndexOf("name", "123"), -1);
    EXPECT_EQ(model_->IndexOf({ { "coord.x", 43 }, { "coord.y", 777 } }), -1);
}

TEST_F(ObjectRefModelF, ItemDataChangedSignal)
{
    QSignalSpy item_data_changed_signal(model_.get(), &ObjectRefModel::itemDataChanged);
    model_->SetItemDataChangedRoles(Role::ALL_ROLES);  // включаем сигналы об изменении всех ролей
    QCoreApplication::processEvents();  // прокручиваем event loop, так как в SetItemDataChangedRoles есть отложенный вызов на обновление подключений к объектам

    model_->AppendVector({ objects_.begin(), objects_.end() });
    const auto& role_ids = model_->roleIds();

    {  // проверяем приход сигнала через event loop
        objects_[0]->SetId(100);
        objects_[1]->SetName("100");
        objects_[2]->SetCoord(new CoordObject(dummy_parent_.get()));
        objects_[2]->GetCoord()->SetX(1);
        objects_[2]->GetCoord()->SetType(new CoordTypeObject(dummy_parent_.get()));
        objects_[2]->GetCoord()->GetType()->SetType(CoordTypeObject::Type::Type3);
        EXPECT_EQ(item_data_changed_signal.count(), 0);  // сигнал не должен отсылаться сразу
        QCoreApplication::processEvents();               // прокручиваем event loop
        ASSERT_EQ(item_data_changed_signal.count(), 1);  // теперь должен прийти всего 1 сигнал
        auto signal_params = item_data_changed_signal.takeFirst()[0].value<om::Ids>();
        EXPECT_EQ(signal_params, om::Ids({ role_ids["id"], role_ids["name"], role_ids["coord"], role_ids["coord.x"], role_ids["coord.y"], role_ids["coord.type"],
                                     role_ids["coord.type.type"] }));  // должны были измениться все эти роли. хоть coord.y мы и не меняли, он изменился когда мы вызвали SetCoord()
    }

    {  // проверяем подключение к свойствам объектам
        objects_[2]->GetCoord()->GetType()->SetType(CoordTypeObject::Type::Type2);
        EXPECT_EQ(item_data_changed_signal.count(), 0);  // не должен был прийти сигнал от прошлых вызовов
        QCoreApplication::processEvents();               // прокручиваем event loop
        ASSERT_EQ(item_data_changed_signal.count(), 1);  // должен прийти всего 1 сигнал
        auto signal_params = item_data_changed_signal.takeFirst()[0].value<om::Ids>();
        EXPECT_EQ(signal_params, om::Ids({ role_ids["coord.type.type"] }));  // должен был измениться только coord.type.type
    }

    model_->SetItemDataChangedRoles(QStringList{ "coord.x", "coord.type.type" });  // теперь будем слушать только эти 2 свойства
    QCoreApplication::processEvents();  // прокручиваем event loop, так как в SetItemDataChangedRoles есть отложенный вызов на обновление подключений к объектам

    {
        // проверяем переподключение вложеных свойств объектов
        auto coord = new CoordObject(dummy_parent_.get());
        coord->SetType(new CoordTypeObject(dummy_parent_.get()));
        objects_[2]->SetCoord(coord);
        objects_[2]->GetCoord()->SetY(100);
        QCoreApplication::processEvents();  // прокручиваем event loop
        auto signal_params = item_data_changed_signal.takeFirst()[0].value<om::Ids>();
        EXPECT_EQ(signal_params, om::Ids({ role_ids["coord.x"], role_ids["coord.type.type"] }));  // должны были измениться эти роли. хоть coord.y мы меняли, он не должен прийти
    }

    {  // проверяем переподключение вложеных свойств объектов
        objects_[2]->GetCoord()->GetType()->SetType(CoordTypeObject::Type::Type3);
        QCoreApplication::processEvents();  // прокручиваем event loop
        auto signal_params = item_data_changed_signal.takeFirst()[0].value<om::Ids>();
        EXPECT_EQ(signal_params, om::Ids({ role_ids["coord.type.type"] }));  // поменялся только coord.type.type
    }

    model_->SetItemDataChangedRoles("name");  // теперь будем слушать только 1 роль
    // тут не будем вызывать QCoreApplication::processEvents, чтобы проверить очередность отложенных вызовов
    {
        objects_[2]->GetCoord()->GetType()->SetType(CoordTypeObject::Type::Type2);
        objects_[0]->SetName("Hello world");
        QCoreApplication::processEvents();               // прокручиваем event loop
        EXPECT_EQ(item_data_changed_signal.count(), 0);  // ничего не пришло, так как новая роль не успела подключиться до вызова SetName, а старые мы уже не отправляем
        objects_[0]->SetName("Hello world 2");
        QCoreApplication::processEvents();  // прокручиваем event loop
        auto signal_params = item_data_changed_signal.takeFirst()[0].value<om::Ids>();
        EXPECT_EQ(signal_params, om::Ids({ role_ids["name"] }));  // поменялся только name
    }

    model_->Remove(objects_[0]);  // удаляем объект из модели, больше сигналы от него не должны приходить
    // тут не будем вызывать QCoreApplication::processEvents, чтобы проверить очередность отложенных вызовов
    {
        objects_[0]->SetName("Hello world");
        QCoreApplication::processEvents();               // прокручиваем event loop
        EXPECT_EQ(item_data_changed_signal.count(), 0);  // ничего не пришло, objects_[0] больше не в моделе
        objects_[1]->SetName("Hello world");
        QCoreApplication::processEvents();  // прокручиваем event loop
        auto signal_params = item_data_changed_signal.takeFirst()[0].value<om::Ids>();
        EXPECT_EQ(signal_params, om::Ids({ role_ids["name"] }));  // а вот от objects_[1] продолжают приходить сигналы
    }

    model_->SetItemDataChangedRoles(QStringList{ "changed", "coord.type.changed" });  // теперь будем слушать сигналы
    QCoreApplication::processEvents();  // прокручиваем event loop, так как в SetItemDataChangedRoles есть отложенный вызов на обновление подключений к объектам

    {
        objects_[1]->EmitChanged();
        objects_[2]->GetCoord()->GetType()->EmitChanged();
        QCoreApplication::processEvents();  // прокручиваем event loop
        auto signal_params = item_data_changed_signal.takeFirst()[0].value<om::Ids>();
        EXPECT_EQ(signal_params, om::Ids({ role_ids["changed"], role_ids["coord.type.changed"] }));  // должны были измениться эти роли
    }
}
