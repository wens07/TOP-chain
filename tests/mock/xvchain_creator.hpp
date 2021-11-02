#pragma once
#include <string>
#include "gtest/gtest.h"
#include "xdb/xdb_factory.h"
#include "xstore/xstore_face.h"
#include "xblockstore/xblockstore_face.h"
#include "xvledger/xvaccount.h"
#include "xvledger/xvblockstore.h"
#include "xvledger/xvledger.h"
#include "xmbus/xmessage_bus.h"
#include "tests/mock/xtestdb.hpp"

namespace top
{
    namespace mock
    {
        class xvchain_creator {
        public:
            xvchain_creator() {
                base::xvchain_t::instance().clean_all(true);

                m_bus = top::make_object_ptr<mbus::xmessage_bus_t>(true, 1000);
                base::xvchain_t::instance().set_xevmbus(m_bus.get());

                m_db = db::xdb_factory_t::create_memdb();
                m_store = store::xstore_factory::create_store_with_static_kvdb(m_db);
                base::xvchain_t::instance().set_xdbstore(m_store.get());

                base::xvblockstore_t * blockstore = store::create_vblockstore(m_store.get());
                base::xvchain_t::instance().set_xblockstore(blockstore);
            }

            void create_blockstore_with_xstore() {
                // base::xvchain_t::instance().clean_all(true);
                // mock::xveventbus_impl* mbus_store = new mock::xveventbus_impl();
                // base::xvchain_t::instance().set_xevmbus(mbus_store);

                // m_store = store::xstore_factory::create_store_with_memdb();
                // base::xvchain_t::instance().set_xdbstore(m_store.get());

                // base::xvblockstore_t * blockstore = store::create_vblockstore(m_store.get());
                // base::xvchain_t::instance().set_xblockstore(blockstore);
            }
            base::xvblockstore_t* get_blockstore() const { return base::xvchain_t::instance().get_xblockstore(); }
            base::xvblkstatestore_t*    get_xblkstatestore() const { return base::xvchain_t::instance().get_xstatestore()->get_blkstate_store(); }
            void clean_all() {
                base::xvchain_t::instance().clean_all(false);
            }
            store::xstore_face_t* get_xstore() const {return m_store.get();}
            const xobject_ptr_t<top::mbus::xmessage_bus_face_t> & get_mbus() const {return m_bus;}
            const std::shared_ptr<db::xdb_face_t> &     get_xdb() const {return m_db;}

        private:
            std::shared_ptr<db::xdb_face_t>      m_db{nullptr};
            xobject_ptr_t<store::xstore_face_t>  m_store{nullptr};
            xobject_ptr_t<mbus::xmessage_bus_face_t> m_bus;
        };
    }
}

