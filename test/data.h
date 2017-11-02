#include <vector>
#include <string>
#include <cstdint>

std::vector<uint8_t> string_to_data(std::string string) {
	std::vector<uint8_t> data;
	for (auto&& byte : string) {
		data.push_back(byte);
	}
	return data;
}

std::vector<uint8_t> empty;
std::string empty_hash_256 = "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a";

std::vector<uint8_t> abc = {'A', 'B', 'C'};
std::string abc_hash_256 = "7fb50120d9d1bc7504b4b7f1888d42ed98c0b47ab60a20bd4a2da7b2c1360efa";

std::vector<uint8_t> hello = string_to_data(R"(Hello world!)");
std::string hello_hash_256 = "d6ea8f9a1f22e1298e5a9506bd066f23cc56001f5d36582344a628649df53ae8";

std::vector<uint8_t> lorem = string_to_data(R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Viverra justo nec ultrices dui sapien eget. Erat pellentesque adipiscing commodo elit at imperdiet. Ullamcorper sit amet risus nullam eget. Ac felis donec et odio pellentesque diam volutpat. Molestie at elementum eu facilisis sed odio. Lectus vestibulum mattis ullamcorper velit sed ullamcorper morbi. Pharetra massa massa ultricies mi. Cursus risus at ultrices mi tempus imperdiet nulla. Aliquet risus feugiat in ante metus dictum at tempor. Integer quis auctor elit sed vulputate mi. Lacus sed viverra tellus in hac habitasse platea. Vulputate enim nulla aliquet porttitor lacus luctus accumsan. Gravida cum sociis natoque penatibus.

Massa tincidunt nunc pulvinar sapien. Lacus vestibulum sed arcu non odio euismod. Ut eu sem integer vitae justo eget magna fermentum. Aliquam eleifend mi in nulla posuere sollicitudin. Quis ipsum suspendisse ultrices gravida. Sit amet tellus cras adipiscing enim. Sollicitudin nibh sit amet commodo nulla facilisi. Scelerisque eleifend donec pretium vulputate sapien nec sagittis aliquam malesuada. Gravida cum sociis natoque penatibus et. At risus viverra adipiscing at in.

Adipiscing elit pellentesque habitant morbi tristique senectus et netus. Nisl condimentum id venenatis a condimentum. Tristique senectus et netus et malesuada fames ac turpis egestas. Netus et malesuada fames ac turpis. Aliquet nec ullamcorper sit amet risus nullam eget. Pellentesque dignissim enim sit amet venenatis urna cursus eget nunc. Et odio pellentesque diam volutpat. Nisl nisi scelerisque eu ultrices vitae. Feugiat scelerisque varius morbi enim. Enim eu turpis egestas pretium. Ante in nibh mauris cursus. Sagittis purus sit amet volutpat consequat mauris nunc congue nisi.

Volutpat diam ut venenatis tellus in. Integer vitae justo eget magna fermentum iaculis eu. Urna molestie at elementum eu. Lobortis mattis aliquam faucibus purus in massa tempor nec. Facilisis mauris sit amet massa vitae. Aliquam faucibus purus in massa tempor. Sed libero enim sed faucibus turpis. Eleifend quam adipiscing vitae proin sagittis nisl rhoncus. Nunc aliquet bibendum enim facilisis. Sed vulputate odio ut enim blandit. Elementum nisi quis eleifend quam adipiscing. Aliquet sagittis id consectetur purus ut faucibus pulvinar.

Diam donec adipiscing tristique risus nec. Nibh sit amet commodo nulla facilisi nullam vehicula ipsum. Ornare arcu dui vivamus arcu felis bibendum ut tristique et. Tristique sollicitudin nibh sit amet commodo. Nec ultrices dui sapien eget mi. Euismod elementum nisi quis eleifend quam adipiscing vitae proin. Morbi tincidunt augue interdum velit euismod. Convallis tellus id interdum velit laoreet id donec ultrices tincidunt. Arcu bibendum at varius vel. Fermentum posuere urna nec tincidunt praesent semper. Sagittis vitae et leo duis. Gravida arcu ac tortor dignissim convallis aenean. Eget lorem dolor sed viverra ipsum. Massa sed elementum tempus egestas sed sed risus. Nibh venenatis cras sed felis eget velit. Sagittis purus sit amet volutpat consequat mauris nunc congue nisi.)");
std::string lorem_hash_256 = "698654e8f16c83f7460b640dcf9da61fe6742e03e98a1afdf880cdff26c95eed";
