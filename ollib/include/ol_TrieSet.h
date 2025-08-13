/****************************************************************************************/
/*
 * ��������ol_trieset.h
 * ��������������Trie�����ַ��������࣬���洢������עֵ�����԰�����
 *          - ����TrieMapʵ�֣��ײ�ʹ��TrieMap<bool>�洢��true��Ϊռλֵ��
 *          - ֧���ַ�������ӡ�ɾ�����������ж�
 *          - �ṩǰ׺ƥ�䣨���/�ǰ׺��ǰ׺Ԫ���б���ģʽƥ�䣨ͨ���'.'��
 *          - ��������TrieMap����һ�£�����ʹ�óɱ�
 * ���ߣ�ol
 * ���ñ�׼��C++17�����ϣ�����ol_TrieMap.h��std::list�����ԣ�
 */
/****************************************************************************************/

#ifndef __OL_TRIESET_H
#define __OL_TRIESET_H 1

#include "ol_TrieMap.h"

namespace ol
{

    /**
     * Trie��ʵ�ֵ��ַ��������ࣨTrieSet��
     * @note ���洢�ַ�����������������ֵ���ײ�����TrieMap<bool>ʵ��
     */
    class TrieSet
    {
    private:
        TrieMap<bool> map; // �ײ�TrieMap����true��Ǽ�����

    public:
        // Ԫ�����
        // ===========================================================================
        /**
         * �򼯺�������ַ������ظ���Ӳ�Ӱ�죩
         * @param key Ҫ��ӵ��ַ���
         */
        void put(const std::string& key)
        {
            map.put(key, true); // ��true��Ϊռλֵ������TrieMap������߼�
        }
        // ===========================================================================

        // Ԫ��ɾ��
        // ===========================================================================
        /**
         * �Ӽ�����ɾ���ַ���
         * @param key Ҫɾ�����ַ���
         */
        void remove(const std::string& key)
        {
            map.remove(key); // ����TrieMap��ɾ���߼�
        }
        // ===========================================================================

        // Ԫ�ز�ѯ
        // ===========================================================================
        /**
         * �ж��ַ����Ƿ�����ڼ�����
         * @param key Ҫ�����ַ���
         * @return ���ڷ���true�����򷵻�false
         */
        bool has(const std::string& key)
        {
            return map.has(key); // ����TrieMap�Ĵ������ж�
        }

        /**
         * ���Ҳ�ѯ�ַ��������ǰ׺����ǰ׺�����Ǽ����е�Ԫ�أ�
         * @param query Ŀ���ַ���
         * @return ���ǰ׺�ַ������������򷵻ؿմ�
         */
        std::string shortestPrefix(const std::string& query)
        {
            return map.shortestPrefix(query); // ����TrieMap��ǰ׺��ѯ
        }

        /**
         * ���Ҳ�ѯ�ַ������ǰ׺����ǰ׺�����Ǽ����е�Ԫ�أ�
         * @param query Ŀ���ַ���
         * @return �ǰ׺�ַ������������򷵻ؿմ�
         */
        std::string longestPrefix(const std::string& query)
        {
            return map.longestPrefix(query); // ����TrieMap��ǰ׺��ѯ
        }

        /**
         * ��ȡ������ָ��ǰ׺��ͷ��Ԫ��
         * @param prefix ǰ׺�ַ���
         * @return ƥ���Ԫ���б�std::list<std::string>��
         */
        std::list<std::string> keysByPrefix(const std::string& prefix)
        {
            return map.keysByPrefix(prefix); // ����TrieMap��ǰ׺ƥ��
        }

        /**
         * �жϼ������Ƿ������ָ��ǰ׺��ͷ��Ԫ��
         * @param prefix ǰ׺�ַ���
         * @return ���ڷ���true�����򷵻�false
         */
        bool hasPrefix(const std::string& prefix)
        {
            return map.hasPrefix(prefix); // ����TrieMap��ǰ׺�������ж�
        }

        /**
         * ��ȡ����ƥ��ģʽ��Ԫ�أ�֧��'.'��Ϊͨ�����ƥ�䵥�������ַ���
         * @param pattern ģʽ�ַ���
         * @return ƥ���Ԫ���б�std::list<std::string>��
         */
        std::list<std::string> keysByPattern(const std::string& pattern)
        {
            return map.keysByPattern(pattern); // ����TrieMap��ģʽƥ��
        }

        /**
         * �жϼ������Ƿ����ƥ��ģʽ��Ԫ�أ�֧��'.'��Ϊͨ�����
         * @param pattern ģʽ�ַ���
         * @return ���ڷ���true�����򷵻�false
         */
        bool hasPattern(const std::string& pattern)
        {
            return map.hasPattern(pattern); // ����TrieMap��ģʽ�������ж�
        }

        /**
         * ��ȡ������Ԫ�ص�����
         * @return Ԫ��������size_t��
         */
        size_t size() const
        {
            return map.size(); // ����TrieMap�ļ���
        }
        // ===========================================================================
    };

} // namespace ol

#endif // !__OL_TRIESET_H