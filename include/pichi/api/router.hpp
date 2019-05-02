#ifndef PICHI_API_ROUTER_HPP
#define PICHI_API_ROUTER_HPP

#include <map>
#include <memory>
#include <pichi/api/vos.hpp>

struct MMDB_s;

namespace boost::asio::ip {

class tcp;
template <typename Protocol> class basic_endpoint;
template <typename Protocol> class basic_resolver_results;

} // namespace boost::asio::ip

namespace pichi::net {

struct Endpoint;

} // namespace pichi::net

namespace pichi::api {

extern bool matchPattern(std::string_view remote, std::string_view pattern);
extern bool matchDomain(std::string_view subdomain, std::string_view domain);

class Geo {
public:
  Geo(char const* fn);
  ~Geo();

  Geo(Geo&&) noexcept = default;
  Geo& operator=(Geo&&) noexcept = default;

  Geo(Geo const&) = delete;
  Geo& operator=(Geo const&) = delete;

public:
  bool match(boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> const&,
             std::string_view country) const;

private:
  std::unique_ptr<MMDB_s> db_;
};

class Router {
public:
  using VO = RuleVO;

private:
  using ResolvedResult = boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp>;
  using Matcher = std::function<bool(net::Endpoint const&, ResolvedResult const&, std::string_view,
                                     AdapterType)>;
  using Container = std::map<std::string, std::pair<RuleVO, std::vector<Matcher>>, std::less<>>;
  using DelegateIterator = typename Container::const_iterator;
  using ValueType = std::pair<std::string_view, RuleVO const&>;
  using ConstIterator = Iterator<DelegateIterator, ValueType>;

  static ValueType generatePair(DelegateIterator);

public:
  Router(char const* fn);

  std::string_view route(net::Endpoint const&, std::string_view ingress, AdapterType,
                         ResolvedResult const&) const;

  void update(std::string const&, RuleVO);
  void erase(std::string_view);

  ConstIterator begin() const noexcept;
  ConstIterator end() const noexcept;
  bool isUsed(std::string_view) const;
  bool needResloving() const;

  RouteVO getRoute() const;
  void setRoute(RouteVO);

private:
  Geo geo_;
  Container rules_ = {};
  bool needResolving_ = false;
  RouteVO route_ = {"direct"};
};

} // namespace pichi::api

#endif // PICHI_API_ROUTER_HPP
