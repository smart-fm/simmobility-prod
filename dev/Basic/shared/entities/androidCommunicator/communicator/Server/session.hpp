
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

namespace sim_mob {

/// The session class provides serialization primitives on top of a socket.
/**
 * Each message sent using this class consists of:
 * 8-byte header containing the length of the serialized data in hexadecimal.
 * and then The serialized data.
 */
class session
{
public:
  /// Constructor.
  session(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  /// Get the underlying socket. Used for making a session or for accepting
  /// an incoming connection.
  boost::asio::ip::tcp::socket& socket()
  {
    return socket_;
  }

  /// Asynchronously write a data structure to the socket.
  template <typename Handler>
  void async_write(std::string &data, Handler handler)
  {
	outbound_data_ = data;
	std::cout << "outbound_data_ is : [" << outbound_data_ << "] " << std::endl;
    // Format the header.
    std::ostringstream header_stream;
    header_stream << std::setw(header_length)
      << std::hex << outbound_data_.size();
    if (!header_stream || header_stream.str().size() != header_length)
    {
      // Something went wrong, inform the caller.
      boost::system::error_code error(boost::asio::error::invalid_argument);
      socket_.get_io_service().post(boost::bind(handler, error));
      return;
    }
    outbound_header_ = header_stream.str();

    std::vector<boost::asio::const_buffer> buffers;
//    buffers.push_back(boost::asio::buffer(outbound_header_));
//    buffers.push_back(boost::asio::buffer(outbound_data_));
    boost::asio::async_write(socket_, boost::asio::buffer(outbound_data_), handler);
  }

  /// Asynchronously read a data structure from the socket.
  template <typename Handler>
  void async_read(std::string &input, Handler handler)
  {
//	  std::vector<char>* t
    // Issue a read operation to read exactly the number of bytes in a header.
    void (session::*f)(const boost::system::error_code&,/*std::vector<char>*,*/ std::string &, boost::tuple<Handler>) = &session::handle_read_header<Handler>;
    boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),boost::bind(f,this, boost::asio::placeholders::error,boost::ref(input)/*, t*/,boost::make_tuple(handler)));


  }

  /// Handle a completed read of a message header. The handler is passed using
  /// a tuple since boost::bind seems to have trouble binding a function object
  /// created using boost::bind as a parameter.
  template <typename Handler>
  void handle_read_header(const boost::system::error_code& e,/*std::vector<char>*t*/std::string &input, boost::tuple<Handler> handler)
  {
    if (e)
    {
      boost::get<0>(handler)(e);
    }
    else
    {
      std::istringstream is(std::string(inbound_header_, header_length));
      std::size_t inbound_data_size = 0;
      if (!(is >> std::hex >> inbound_data_size))
      {
        std::cout << "ERROR in session-Handle_read_header" << std::endl;
        return;
      }
      inbound_data_.resize(inbound_data_size);

      void (session::*f)(const boost::system::error_code&,/*std::vector<char>**/std::string &, boost::tuple<Handler>) = &session::handle_read_data<Handler>;
      boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),boost::bind(f, this,boost::asio::placeholders::error, /*t,*/ boost::ref(input), handler));
    }
  }

  /// Handle a completed read of message data.
  template <typename Handler>
  void handle_read_data(const boost::system::error_code& e,/*std::vector<char> * t*/std::string &input, boost::tuple<Handler> handler)
  {
	  if(e)
    {
      boost::get<0>(handler)(e);
    }
    else
    {
      try
      {
    	  std::string archive_data(&inbound_data_[0], inbound_data_.size());
    	  std::cout << "4.inbound_data_[" << archive_data << "]" << std::endl;
    	  input = archive_data;
      }
      catch (std::exception& e)
      {
        // Unable to decode data.
        boost::system::error_code error(boost::asio::error::invalid_argument);
        boost::get<0>(handler)(error);
        return;
      }
      boost::get<0>(handler)(e);
    }
  }

private:
  /// The underlying socket.
  boost::asio::ip::tcp::socket socket_;

  /// The size of a fixed length header.
  enum { header_length = 8 };

  /// Holds an outbound header.
  std::string outbound_header_;

  /// Holds the outbound data.
  std::string outbound_data_;

  /// Holds an inbound header.
  char inbound_header_[header_length];

  /// Holds the inbound data.
  std::vector<char> inbound_data_;
};

typedef boost::shared_ptr<session> session_ptr;

} // namespace sim_mob

