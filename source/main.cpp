#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <functional>
#include <unordered_map>

/// <summary>
/// Parse Argument Like:
/// -A
/// -AB(will parse to -A -B)
/// --Test
/// -A=1
/// --Test=2
/// -A 5
/// --Test 3
/// </summary>
class Argument
{
public:
	enum class Exception
	{
		none,
		denseOptionAssign,
		unallowedOptionName
	};
private:
	// named option
	std::unordered_map<std::string, std::vector<std::string>> options;
	// unnamed args
	std::vector<std::string> args;
	Exception except = Exception::none;
public:
	Argument(int argc, char** argv, const std::unordered_map<std::string, std::string>& replace = std::unordered_map<std::string, std::string>()) : Argument(argc, (const char**) argv, replace)
	{}
	Argument(int argc, const char** argv, const std::unordered_map<std::string, std::string>& replace = std::unordered_map<std::string, std::string>())
	{
		if (argc < 1) throw std::invalid_argument("arguments too few");
		options["program path"].emplace_back(argv[0]);

		std::vector<std::string> temp;
		for (size_t i = 1; i < argc; i++)
			temp.emplace_back(argv[i]);

		auto fun0 = [&replace](const std::string& str) {
			auto it = replace.find(str);
			if (it == replace.end()) return str;
			return it->second;
		};
		auto fun1 = [](char c) {
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
		};
		auto fun2 = [&fun1](const std::string& s) {
			for (size_t i = 0; i < s.size(); i++)
			{
				auto c = s[i];
				if ((!i || i == s.size() - 1) && !fun1(c)) return false;
				if (!(fun1(c) || c == '.' || c == '-' || c == '_' || std::isdigit(c))) return false;
			}
			return true;
		};

		bool beforeNoValOpt = false;
		std::string last;
		for (const auto& i : temp)
		{
			uint8_t dashLen = i.starts_with("--") ? 2 : i.starts_with("-");
			std::string rest = i.substr(dashLen);
			size_t len = rest.size();
			bool assign = false;
			for (size_t i = 0; i < len; i++) if (rest[i] == '=') assign = len = i;
			if (dashLen == 1)
			{
				// len > 1 indicates multiple options for abbreviations, so can't assign for them
				if (len > 1 && assign)
				{
					except = Exception::denseOptionAssign;
					return;
				}
				for (size_t j = 0; j < len; j++)
				{
					auto k = rest[j];
					if (!fun1(k))
					{
						except = Exception::unallowedOptionName;
						return;
					}
					options[last = fun0(std::string(&k, 1))];
				}
				if (len == 1)
				{
					beforeNoValOpt = !assign;
					if (assign)
						options[last].push_back(rest.substr(len + 1));
				}
			}
			else if (dashLen == 2)
			{
				if (!fun2(last = fun0(rest.substr(0, len))))
				{
					except = Exception::unallowedOptionName;
					return;
				}
				options[last];
				beforeNoValOpt = !assign;
				if (assign)
					options[last].push_back(rest.substr(len + 1));
			}
			else if (beforeNoValOpt)
			{
				options[last].push_back(rest);
				beforeNoValOpt = false;
			}
			else
			{
				args.push_back(rest);
			}
		}
	}

	void OnException(const std::function<void(Exception)>& func)
	{
		if (except == Exception::none) return;
		func(except);
		except = Exception::none;
	}
	bool OptionExist(const std::string& name) const
	{
		if (except != Exception::none) throw std::runtime_error("There are unprocessed exceptions present");
		return options.find(name) != options.end();
	}
	std::optional<std::vector<std::string>> GetOption(const std::string& name) const
	{
		if (except != Exception::none) throw std::runtime_error("There are unprocessed exceptions present");
		auto it = options.find(name);
		if (it == options.end()) return std::nullopt;
		return it->second;
	}
	std::vector<std::string> GetUnnamedArgs() const
	{
		if (except != Exception::none) throw std::runtime_error("There are unprocessed exceptions present");
		return args;
	}
};

int main(int argc, char** argv)
{
	Argument arg(argc, argv);
	arg.OnException([](auto i) {
		
	});
	return 0;
}