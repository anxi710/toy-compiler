#include "preproc.hpp"

namespace preproc {

/**
 * @brief  删除输入字符流中的注释
 * @param  text 输入字符串
 * @return 删除后的字符串
 */
std::string
removeAnnotations(std::string text)
{
  std::string result{}; // 删除注释后的字符串
  std::size_t i{};      // index
  int depth{};          // 嵌套深度

  while (i < text.length()) {
    if (depth == 0 && text[i] == '/' && i + 1 < text.length()) {
      if (text[i + 1] == '/') {
        // 单行注释，跳过直到换行
        i += 2;
        while (i < text.length() && text[i] != '\n') {
          i++;
        }
      } else if (text[i + 1] == '*') {
        // 块注释开始
        depth++;
        i += 2;
      } else {
        result += text[i++];
      }
    } else if (depth > 0) {
      if (text[i] == '/' && i + 1 < text.length() && text[i + 1] == '*') {
        // 嵌套块注释
        depth++;
        i += 2;
      } else if (text[i] == '*' && i + 1 < text.length() && text[i + 1] == '/') {
        // 块注释结束
        depth--;
        i += 2;
      } else {
        if (text[i] == '\n') {
          result += text[i]; // 保留换行
        }
        i++;
      }
    } else {
      result += text[i++];
    }
  } // end while

  return result;
}

} // namespace preproc
