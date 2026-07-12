//
// Created by Dennis on 13.10.2025.
//
#pragma once
#include <iostream>
#include <string>
#include <map>
#include <ranges>
#include <set>
#include <vector>

namespace Setup
{
    using std::string;
    using std::vector;
    using std::set;
    using std::map;
    using std::endl;
    using std::cout;
    using std::views::enumerate;


    class ArgumenteAuswerten
    {
    private:
        const vector<string>& _inputArgumente;
        const std::map<string, string>& _allowed_keys;
        vector<std::string> _ignored_keys;
        std::map<string, string> _valid_keys;

        void print_accepted_keys(const std::map<string, string>& _allowed_args,
                                 vector<std::string>& _ignored_keys,
                                 std::map<string, string>& _valid_keys);

    public:
        /**
         * @brief Konstruktor
         * @param input_argumente
         * @param allowed_keys
         */
        explicit ArgumenteAuswerten(const std::vector<std::string>& input_argumente,
                                    const std::map<std::string, std::string>& allowed_keys);

        static bool check_and_insert(string const& key, map<string, string>& map);

        void Collect_Arguments(const vector<string>& input_argumente/*, map<string,string> & options*/);

        void print_Input_Argumente(const vector<string>& input_argumente) const;

        static void print_Erlaubte_Optionen(const std::map<std::string,std::string>& allowed_args);


        void print_accepted_keys(const std::map<std::string, std::string>&,
                                 std::set<std::string>&,
                                 std::set<std::string>&) const;

        void print_ignored_keys(const std::map<string, string>&) const;
        vector<string> get_InputArgumente() const;
        std::map<string, string> get_accepted_arguments() const;
        std::map<string, string> get_allowed_keys() const;
        vector<string> get_ignored_keys() const;
    };

    /*// Ignorierte Keys (Differenz)
    std::set_difference(Argumente.get_accepted_arguments().begin(), Argumente.get_accepted_arguments().end(),
                       allowed_keys.begin(), allowed_keys.end(),
                       std::inserter(ignored_keys, ignored_keys.begin()));

    // Gültige Keys (Schnittmenge)
    std::set_intersection(Argumente.get_accepted_arguments().begin(), Argumente.get_accepted_arguments().end(),
                         allowed_keys.begin(), allowed_keys.end(),
                         std::inserter(valid_keys, valid_keys.begin()));
*/
    /* Verwendungsbeispiel:
    if (Argumente.get_accepted_arguments().contains("verbose")) {
        std::cout << "Verbose-Modus aktiviert" << std::endl;
    }
    if (Argumente.get_accepted_arguments().contains("output")) {
        std::cout << "Ausgabe wird in " << Argumente.get_accepted_arguments()["output"] << " geschrieben" << std::endl;
    }*/
} // namespace Setup

/*
// Ausgabe der Argumente
Argumente.print_Input_Argumente(argumente);

// Ausgabe der erlaubten Optionen
Argumente.print_Erlaubte_Optionen(allowed_args);
*/