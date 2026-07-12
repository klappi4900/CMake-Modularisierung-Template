//
// Created by Dennis on 13.10.2025.
//
#include "../includes/ArgumenteAuswerten.h"

namespace Setup
{
    ArgumenteAuswerten::ArgumenteAuswerten(const vector<string>& inputsArgumente,
                                           const map<std::string, std::string>& allowed_keys) :
        _inputArgumente(inputsArgumente), _allowed_keys(allowed_keys)
    {
        Collect_Arguments(_inputArgumente);
    }

    bool ArgumenteAuswerten::check_and_insert(string const& key, map<string, string>& map)
    {
        if (map.contains(key))
        {
            return false;
        }
        else
        {
            return true;
        }
    };

    void ArgumenteAuswerten::Collect_Arguments(const vector<string>& input_argumente)
    {
        for (unsigned int i = 0; i < input_argumente.size(); ++i)
        {
            const std::string& input_argument = input_argumente[i];


            // Lange Optionen mit Wert: --option=value
            if (input_argument.substr(0, 2) == "--" && input_argument.find('=') != std::string::npos)
            {
                unsigned int equalsPos = input_argument.find('=');
                string key = input_argument.substr(2, equalsPos - 2);
                string value = input_argument.substr(equalsPos + 1);
                if (check_and_insert(key, this->_valid_keys))
                {
                    this->_valid_keys[key] = value;
                }
                else
                {
                    std::cerr << "Key: " << key << " bereits vorhanden" << std::endl;
                    _ignored_keys.push_back(key);
                    continue;
                }
            }
            // Lange Optionen ohne Wert: --option value
            else if (input_argument.substr(0, 2) == "--" && i + 1 < input_argumente.size() && input_argumente[i + 1][0]
                != '-')
            {
                std::string key = input_argument.substr(2);
                const std::string& value = input_argumente[i + 1];
                if (check_and_insert(key, this->_valid_keys))
                {
                    this->_valid_keys[key] = value;
                }
                else
                {
                    _ignored_keys.push_back(key);
                }
                ++i; // Nächstes Argument überspringen, da es als Wert verwendet wurde
                continue;
            }
            // Lange Optionen ohne Wert: --option (als Flag)
            else if (input_argument.substr(0, 2) == "--" && input_argument.find('=') == std::string::npos)
            {
                std::string key = input_argument.substr(2);
                if (check_and_insert(key, this->_valid_keys))
                {
                    this->_valid_keys[key] = "LongSchalter"; // Flag setzen
                }
                else
                {
                    _ignored_keys.push_back(key);
                }
                continue;
            }

            // Kurze Optionen: -o value
            if (input_argument[0] == '-' && input_argument.length() == 2 && i + 1 < input_argumente.size() &&
                input_argumente[i + 1][0] != '-')
            {
                std::string key = input_argument.substr(1);
                const std::string& value = input_argumente[i + 1];
                if (check_and_insert(key, this->_valid_keys))
                {
                    this->_valid_keys[key] = value;
                }
                else
                {
                    _ignored_keys.push_back(key);
                }
                ++i; // Nächstes Argument überspringen, da es als Wert verwendet wurde
                continue;
            }
            // Kurze Optionen: -o (als Flag)
            else if (input_argument[0] == '-' && input_argument.length() > 1 && input_argument[1] != '-' &&
                input_argument[2] != '=')
            {
                std::string key = input_argument.substr(1);
                if (check_and_insert(key, this->_valid_keys))
                {
                    this->_valid_keys[key] = "shortSchalter"; // Flag setzen
                }
                else
                {
                    _ignored_keys.push_back(key);
                }
                continue;
            }
            // Kurze Optionen: -p=value
            if (input_argument.substr(0, 1) == "-" && input_argument.find('=') != std::string::npos && input_argument[1]
                != '-')
            {
                size_t equalsPos = input_argument.find('=');
                std::string key = input_argument.substr(1, equalsPos - 1);
                std::string value = input_argument.substr(equalsPos + 1);
                if (check_and_insert(key, this->_valid_keys))
                {
                    this->_valid_keys[key] = value;
                }
                else
                {
                    _ignored_keys.push_back(key);
                }
            }
        }
    }

    void ArgumenteAuswerten::print_Input_Argumente(const vector<string>& input_argumente) const
    {
        // Ausgabe aller erkannten Optionen:
        std::cout << "Erhaltene Argumente:" << std::endl;
        for (const auto& [key, value] : this->get_accepted_arguments())
        {
            std::cout << "  " << key << " = " << value << std::endl;
        }
    }

    void ArgumenteAuswerten::print_Erlaubte_Optionen(const std::map<std::string, std::string>& allowed_args)
    {
        // Ausgabe aller erlaubte Optionen:
        std::cout << "\nErlaubte Optionen:" << std::endl;
        for (auto&& [i, key] : enumerate(allowed_args)) {
            std::cout << key.first;
            if (i == allowed_args.size() - 1) {
                std::cout << std::endl;
            } else std::cout << ", ";
        }
        std::cout << std::endl;
        std::cout << "\n" << std::endl;
    }

    void ArgumenteAuswerten::print_accepted_keys(
        const std::map<std::string,std::string>& allowed_args,
        std::set<std::string>& ignored_keys,
        std::set<std::string>& valid_keys) const
    {
        for (const auto& [key,value] : this->get_accepted_arguments())
        {
            if (allowed_args.find(key) == allowed_args.end())
            {
                ignored_keys.insert(key);
            }
            else
            {
                valid_keys.insert(key);
            }
        }
    }

    void ArgumenteAuswerten::print_ignored_keys(const std::map<string, string>&) const
    {
        for (const auto& key : this->get_ignored_keys()) {
            std::cout << key << ", " << std::flush;
        }
        std::cout << "\n" << std::endl;
    }

    vector<string> ArgumenteAuswerten::get_InputArgumente() const
    {
        return _inputArgumente;
    }


    std::map<string, string> ArgumenteAuswerten::get_accepted_arguments() const
    {
        return this->_valid_keys;
    }

    std::map<string, string> ArgumenteAuswerten::get_allowed_keys() const
    {
        return this->_allowed_keys;
    }

    vector<string> ArgumenteAuswerten::get_ignored_keys() const
    {
        return this->_ignored_keys;
    };
} // namespace Setup
