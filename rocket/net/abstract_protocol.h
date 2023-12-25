#ifndef ROCKET_NET_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_ABSTRACT_PROTOCOL_H

#include <memory>


namespace rocket {
// enable_shared_from_this: 用于帮助在类中获取指向自身的 std::shared_ptr
// 对象指针->shared_from_this() 即可获得自身的　智能指针
class AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol>{
   public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

    std::string getReqId() {
        return m_req_id;
    }

    void setReqId(const std::string& req_id) {
        m_req_id = req_id;
    }

    // 只要基类的析构函数是虚拟的，派生类的析构函数就会自动地成为虚函数
    virtual ~AbstractProtocol() {}


   protected:
    std::string m_req_id;  // 请求号,唯一标识一个请求或响应
};
}  // namespace rocket

#endif
