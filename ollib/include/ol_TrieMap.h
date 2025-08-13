#ifndef __OL_TRIEMAP_H
#define __OL_TRIEMAP_H 1

#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace ol
{

    template <typename V>
    class TrieNode
    {
    public:
        V val = V();                                                              // �ڵ�洢��ֵ��Ĭ�ϳ�ʼ��
        bool isValid = false;                                                     // ��Ǹýڵ��Ƿ�洢����Ч����
        std::unordered_map<unsigned char, std::shared_ptr<TrieNode<V>>> children; // ʹ������ӳ��洢�ӽڵ㣬��Ϊ�ַ���ֵΪ�ӽڵ�ָ��

        TrieNode()
        {
        } // ����Ԥ����ռ䣬���캯����
    };

    template <typename V>
    class TrieMap
    {
    private:
        std::shared_ptr<TrieNode<V>> root; // Trie ���ĸ��ڵ�
        size_t count;                      // ��ǰ���ڵļ�ֵ�Ը���

        /**
         * ������nodeΪ����Trie�����ռ�����������
         * @param node ��ʼ�ڵ�
         * @param path ��ǰ·�����ѱ������ַ���
         * @param res �洢������б�
         */
        void traverse(std::shared_ptr<TrieNode<V>> node, std::string& path, std::list<std::string>& res)
        {
            if (!node) return;

            if (node->isValid) // �ñ��λ�ж��Ƿ�Ϊ��Ч��
            {
                res.push_back(path);
            }

            for (auto& [c, child] : node->children)
            {
                path.push_back(c);
                traverse(child, path, res);
                path.pop_back();
            }
        }

        /**
         * ��ģʽƥ�����Trie��
         * @param node ��ʼ�ڵ�
         * @param path ��ǰ·������ƥ����ַ���
         * @param pattern ģʽ�ַ�����֧��'.'��Ϊͨ���
         * @param i ��ǰƥ�䵽��ģʽλ��
         * @param res �洢ƥ�������б�
         */
        void traverseByPattern(std::shared_ptr<TrieNode<V>> node, std::string& path,
                               const std::string& pattern, size_t i, std::list<std::string>& res)
        {
            if (!node) return; // �ڵ㲻���ڣ�ƥ��ʧ��

            // ģʽƥ�����
            if (i == pattern.length())
            {
                // �����ǰ�ڵ�Ϊ��Ч��
                if (node->isValid)
                {
                    res.push_back(path);
                }
                return;
            }

            unsigned char c = pattern[i];
            if (c == '.')
            {
                // ͨ���ƥ�������ַ����������п��ܵ��ӽڵ�
                for (auto& [ch, child] : node->children)
                {
                    path.push_back(ch);
                    traverseByPattern(child, path, pattern, i + 1, res);
                    path.pop_back();
                }
            }
            else
            {
                // ��ͨ�ַ������Ҷ�Ӧ���ӽڵ�
                auto it = node->children.find(c);
                if (it != node->children.end())
                {
                    path.push_back(c);
                    traverseByPattern(it->second, path, pattern, i + 1, res);
                    path.pop_back();
                }
            }
        }

        /**
         * �ݹ�����ֵ��
         * @param node ��ǰ�ڵ�
         * @param key Ҫ����ļ�
         * @param val Ҫ�����ֵ
         * @param i ��ǰ����ļ���λ��
         * @return �����Ľڵ㣨�������½��ģ�
         */
        std::shared_ptr<TrieNode<V>> put(std::shared_ptr<TrieNode<V>> node, const std::string& key, V val, size_t i)
        {
            if (!node)
            {
                node = std::make_shared<TrieNode<V>>();
            }

            if (i == key.length())
            {
                node->val = val;
                node->isValid = true; // ���Ϊ��Ч�ڵ㣨����val�Ƿ�ΪĬ��ֵ��
                return node;
            }

            unsigned char c = key[i];
            node->children[c] = put(node->children[c], key, val, i + 1);
            return node;
        }

        /**
         * �ݹ�ɾ����
         * @param node ��ǰ�ڵ�
         * @param key Ҫɾ���ļ�
         * @param i ��ǰ����ļ���λ��
         * @return ɾ�������Ľڵ㣨����Ϊnullptr��
         */
        std::shared_ptr<TrieNode<V>> remove(std::shared_ptr<TrieNode<V>> node, const std::string& key, size_t i)
        {
            if (!node) return nullptr; // �ڵ㲻���ڣ�ֱ�ӷ���

            // �������ĩβ�����Ϊ��Ч
            if (i == key.length())
            {
                node->isValid = false;
            }
            else
            {
                unsigned char c = key[i];
                // �ݹ�ɾ���ӽڵ�
                node->children[c] = remove(node->children[c], key, i + 1);
            }

            // ����������ڵ���Ч��isValid=true���������ڵ�
            if (node->isValid)
            {
                return node;
            }

            // ���û���ӽڵ㣬ɾ����ǰ�ڵ�
            if (node->children.empty())
            {
                return nullptr;
            }

            // ���ӽڵ㵫��ֵ�������ڵ���Ϊ�м�·��
            return node;
        }

        /**
         * ���ģʽ�Ƿ�ƥ��
         * @param node ��ǰ�ڵ�
         * @param pattern ģʽ�ַ���
         * @param i ��ǰ�����ģʽλ��
         * @return �Ƿ����ƥ��ļ�
         */
        bool hasPattern(std::shared_ptr<TrieNode<V>> node, const std::string& pattern, size_t i)
        {
            if (!node) return false; // �ڵ㲻���ڣ�ƥ��ʧ��

            // ģʽ������ϣ���鵱ǰ�ڵ��Ƿ���ֵ
            if (i == pattern.length())
            {
                return node->isValid;
            }

            unsigned char c = pattern[i];
            if (c != '.')
            {
                // ��ͨ�ַ������Ҷ�Ӧ�ӽڵ㲢�ݹ�ƥ��
                auto it = node->children.find(c);
                return (it != node->children.end()) && hasPattern(it->second, pattern, i + 1);
            }
            else
            {
                // ͨ��������������ӽڵ�
                for (auto& [ch, child] : node->children)
                {
                    if (hasPattern(child, pattern, i + 1))
                    {
                        return true;
                    }
                }
                return false;
            }
        }

        /**
         * ���Ҽ���Ӧ�Ľڵ�
         * @param node ��ʼ�ڵ�
         * @param key Ҫ���ҵļ�
         * @return ��ĩβ��Ӧ�Ľڵ㣬�������򷵻�nullptr
         */
        std::shared_ptr<TrieNode<V>> findNode(std::shared_ptr<TrieNode<V>> node, const std::string& key)
        {
            auto current = node;
            for (size_t i = 0; i < key.length(); ++i)
            {
                if (!current) return nullptr; // ·���жϣ�����null

                // ������һ���ַ���Ӧ���ӽڵ�
                auto it = current->children.find(key[i]);
                if (it == current->children.end())
                {
                    return nullptr; // �ӽڵ㲻���ڣ�����null
                }

                current = it->second;
            }
            return current; // ���ؼ�ĩβ��Ӧ�Ľڵ�
        }

    public:
        /**
         * ���캯������ʼ��Trie��
         */
        TrieMap() : root(std::make_shared<TrieNode<V>>()), count(0)
        {
        }

        /**
         * �������¼�ֵ��
         * @param key Ҫ����ļ�
         * @param val Ҫ�����ֵ
         */
        void put(const std::string& key, V val)
        {
            // ������¼������Ӽ���
            if (!has(key))
            {
                ++count;
            }
            root = put(root, key, val, 0);
        }

        /**
         * ɾ����ֵ��
         * @param key Ҫɾ���ļ�
         */
        void remove(const std::string& key)
        {
            if (!has(key))
            {
                return;
            }
            // �ݹ�ɾ�������¸��ڵ�
            root = remove(root, key, 0);
            --count;
        }

        /**
         * ��ȡ����Ӧ��ֵ��ͨ�� std::optional ��ȷ��ʾֵ�Ƿ����
         * @param key Ҫ��ѯ�ļ�
         * @return �������ڣ����ذ�����Ӧֵ�� std::optional<V>�����������ڣ����� std::nullopt
         * @note ��ͨ�� has_value() �жϼ��Ƿ���ڣ�ͨ�� value() ��ȡֵ��������ʱ��
         */
        std::optional<V> get(const std::string& key)
        {
            auto node = findNode(root, key);
            if (node && node->isValid)
            {
                return node->val;
            }
            return std::nullopt;
        }

        /**
         * �����Ƿ����
         * @param key Ҫ���ļ�
         * @return �����ڷ���true�����򷵻�false
         */
        bool has(const std::string& key)
        {
            auto node = findNode(root, key);
            return (node && node->isValid);
        }

        /**
         * ����Ƿ������ָ��ǰ׺��ͷ�ļ�
         * @param prefix ǰ׺�ַ���
         * @return ���ڷ���true�����򷵻�false
         */
        bool hasPrefix(const std::string& prefix)
        {
            return findNode(root, prefix) != nullptr;
        }

        /**
         * ���Ҳ�ѯ�ַ��������ǰ׺��
         * @param query ��ѯ�ַ���
         * @return ���ǰ׺�����������򷵻ؿ��ַ���
         */
        std::string shortestPrefix(const std::string& query)
        {
            auto current = root;
            for (size_t i = 0; i < query.length(); ++i)
            {
                if (!current) break;

                // ����isValid�жϣ�����val�Ƿ�ΪĬ��ֵ
                if (current->isValid)
                {
                    return query.substr(0, i);
                }

                auto it = current->children.find(query[i]);
                if (it == current->children.end()) break;
                current = it->second;
            }

            if (current && current->isValid)
            {
                return query;
            }

            return "";
        }

        /**
         * ���Ҳ�ѯ�ַ������ǰ׺��
         * @param query ��ѯ�ַ���
         * @return �ǰ׺�����������򷵻ؿ��ַ���
         */
        std::string longestPrefix(const std::string& query)
        {
            auto current = root;
            size_t max_len = 0; // ��¼���Чǰ׺�ĳ���
            size_t i;           // ѭ����������ѭ���ⶨ�壬�����ж��Ƿ�����������ַ���

            for (i = 0; i < query.length(); ++i)
            {
                if (!current) break; // ·���жϣ��˳�ѭ��

                // �����ǰ�ڵ�����Ч���������ǰ׺����
                if (current->isValid)
                {
                    max_len = i;
                }

                // ������һ���ַ���Ӧ���ӽڵ�
                auto it = current->children.find(query[i]);
                if (it == current->children.end()) break; // �ַ���ƥ�䣬�˳�ѭ��
                current = it->second;                     // �ƶ����ӽڵ�
            }

            // ֻ�е���1. �������ѯ�ַ����������ַ���2. ���սڵ�����Ч��
            // �ŷ���������ѯ�ַ�����˵����ѯ�ַ�����������Ѳ���ļ���
            if (i == query.length() && current && current->isValid)
            {
                return query;
            }

            // ���򷵻����Чǰ׺
            return query.substr(0, max_len);
        }

        /**
         * ��ȡ������ָ��ǰ׺��ͷ�ļ�
         * @param prefix ǰ׺�ַ���
         * @return ƥ��ļ��б�
         */
        std::list<std::string> keysByPrefix(const std::string& prefix)
        {
            std::list<std::string> res;
            auto node = findNode(root, prefix);
            if (!node)
            {
                return res;
            }

            // ��ǰ׺�ڵ㿪ʼ���������ӽڵ�
            std::string path = prefix;
            traverse(node, path, res);
            return res;
        }

        /**
         * ��ȡ����ƥ��ģʽ�ļ���֧��'.'��Ϊͨ�����ƥ�䵥�������ַ���
         * @param pattern ģʽ�ַ���������'.'����ƥ�����ⵥ���ַ�
         * @return ƥ��ļ��б�
         */
        std::list<std::string> keysByPattern(const std::string& pattern)
        {
            std::list<std::string> res;
            std::string path;
            traverseByPattern(root, path, pattern, 0, res);
            return res;
        }

        /**
         * ����Ƿ����ƥ��ģʽ�ļ���֧��'.'��Ϊͨ�����ƥ�䵥�������ַ���
         * @param pattern ģʽ�ַ���
         * @return ���ڷ���true�����򷵻�false
         */
        bool hasPattern(const std::string& pattern)
        {
            return hasPattern(root, pattern, 0);
        }

        /**
         * ��ȡ��ǰ��ֵ�Ե�����
         * @return ��ֵ������
         */
        size_t size() const
        {
            return count;
        }
    };

} // namespace ol

#endif // !__OL_TRIEMAP_H