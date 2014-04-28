/*
 * ParameterManager.hpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Max
 */

#pragma once

#include "XmlRpcValue.h"

#include <string>

namespace sim_mob {

using namespace std;

class ParameterManager {
public:
	ParameterManager();
	virtual ~ParameterManager();

public:
   /** \brief Assign value from parameter pool, with default.
	*
	* This method tries to retrieve the indicated parameter value from the
	* parameter pool, storing the result in paramVal.  If the value
	* cannot be retrieved from the server, default_val is used instead.
	*
	* \param paramName The key to be searched on the parameter server.
	* \param[out] paramVal Storage for the retrieved value.
	* \param defaultVal Value to use if the server doesn't contain this
	* parameter.
	* \throws InvalidNameException If the parameter key begins with a tilde, or is an otherwise invalid graph resource name
	*/
	template<typename T>
	void param(const std::string& paramName, T& paramVal, const T& defaultVal) const
	{
		if (hasParam(param_name))
		{
		  if (getParam(param_name, paramVal))
		  {
			return;
		  }
		}

		paramVal = defaultVal;
	}
	/** \brief Check whether a parameter exists in parameter pool
	*
	* \param key The key to check.
	*
	* \return true if the parameter exists, false otherwise
	* \throws InvalidNameException If the parameter key begins with a tilde, or is an otherwise invalid graph resource name
	*/
	bool hasParam(const std::string& key) const;
	/** \brief Get a double value from the parameter pool.
	   *
	   * If you want to provide a default value in case the key does not exist use param().
	   *
	   * \param key The key to be used in the parameter server's dictionary
	   * \param[out] d Storage for the retrieved value.
	   *
	   * \return true if the parameter value was retrieved, false otherwise
	   * \throws InvalidNameException If the parameter key begins with a tilde, or is an otherwise invalid graph resource name
	   */
	  bool getParam(const std::string& key, double& d) const;
	/** \brief Get a string value from the parameter pool
	*
	* \param key The key to be used in the parameter server's dictionary
	* \param[out] s Storage for the retrieved value.
	*
	* \return true if the parameter value was retrieved, false otherwise
	* \throws InvalidNameException If the parameter key begins with a tilde, or is an otherwise invalid graph resource name
	*/
	bool getParam(const std::string& key, std::string& s) const;
	/** \brief Get an integer value from the parameter server.
	   *
	   * If you want to provide a default value in case the key does not exist use param().
	   *
	   * \param key The key to be used in the parameter server's dictionary
	   * \param[out] i Storage for the retrieved value.
	   *
	   * \return true if the parameter value was retrieved, false otherwise
	   * \throws InvalidNameException If the parameter key begins with a tilde, or is an otherwise invalid graph resource name
	   */
	  bool getParam(const std::string& key, int& i) const;
	  /** \brief Get a boolean value from the parameter server.
	   *
	   * If you want to provide a default value in case the key does not exist use param().
	   *
	   * \param key The key to be used in the parameter server's dictionary
	   * \param[out] b Storage for the retrieved value.
	   *
	   * \return true if the parameter value was retrieved, false otherwise
	   * \throws InvalidNameException If the parameter key begins with a tilde, or is an otherwise invalid graph resource name
	   */
	  bool getParam(const std::string& key, bool& b) const;
	  /** \brief Get an arbitrary XML/RPC value from the parameter server.
	   *
	   * If you want to provide a default value in case the key does not exist use param().
	   *
	   * \param key The key to be used in the parameter server's dictionary
	   * \param[out] v Storage for the retrieved value.
	   *
	   * \return true if the parameter value was retrieved, false otherwise
	   * \throws InvalidNameException If the parameter key begins with a tilde, or is an otherwise invalid graph resource name
	   */
	  bool getParam(const std::string& key, XmlRpc::XmlRpcValue& v) const;

private:
	  /**
	   *  \brief store parameters
	   *  \map key is xml Tag or Element "name"
	   *  \map value is XmlRpcValue
	   */
	  std::map<string,XmlRpc::XmlRpcValue> parameterPool;
};

}// namespace sim_mob
