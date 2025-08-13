#ifndef __OL_TRIESET_H
#define __OL_TRIESET_H 1

#include "ol_TrieMap.h"

namespace ol
{

    /**
     * TrieSet ���� TrieMap ʵ�ֵļ��ϣ����洢��������עֵ
     * �ڲ�ʹ�� TrieMap<bool> ��Ϊ�ײ�ṹ���� true ��Ϊռλֵ
     * �������� TrieMap ����һ�£�����ʹ�úͼ���
     */
    class TrieSet
    {
    private:
        TrieMap<bool> map; // �ײ����� TrieMap��ֵ����Ϊ���ڱ��

    public:
        // **** �� ****

        /**
         * �򼯺������Ԫ�أ��ݵȲ������ظ���Ӳ�Ӱ�죩
         * @param key Ҫ��ӵ�Ԫ�أ��ַ�����
         */
        void put(const std::string& key)
        {
            map.put(key, true); // �� true ��Ǽ����ڣ����� TrieMap �� put ����
        }

        // **** ɾ ****

        /**
         * �Ӽ�����ɾ��Ԫ��
         * @param key Ҫɾ����Ԫ�أ��ַ�����
         */
        void remove(const std::string& key)
        {
            map.remove(key); // ���� TrieMap �� remove ����
        }

        // **** �� ****

        /**
         * �ж�Ԫ���Ƿ�����ڼ�����
         * @param key Ҫ����Ԫ�أ��ַ�����
         * @return ���ڷ��� true�����򷵻� false
         */
        bool has(const std::string& key)
        {
            return map.has(key); // ���� TrieMap �� has ����
        }

        /**
         * ���� query �����ǰ׺����ǰ׺��Ϊ�����е�Ԫ�أ�
         * @param query Ŀ���ַ���
         * @return ���ǰ׺�ַ������������򷵻ؿմ�
         */
        std::string shortestPrefix(const std::string& query)
        {
            return map.shortestPrefix(query);
        }

        /**
         * ���� query ���ǰ׺����ǰ׺��Ϊ�����е�Ԫ�أ�
         * @param query Ŀ���ַ���
         * @return �ǰ׺�ַ������������򷵻ؿմ�
         */
        std::string longestPrefix(const std::string& query)
        {
            return map.longestPrefix(query);
        }

        /**
         * ��ȡ������ prefix Ϊǰ׺��Ԫ��
         * @param prefix ǰ׺�ַ���
         * @return ƥ���Ԫ���б�
         */
        std::list<std::string> keysByPrefix(const std::string& prefix)
        {
            return map.keysByPrefix(prefix);
        }

        /**
         * �жϼ������Ƿ������ prefix Ϊǰ׺��Ԫ��
         * @param prefix ǰ׺�ַ���
         * @return ���ڷ��� true�����򷵻� false
         */
        bool hasPrefix(const std::string& prefix)
        {
            return map.hasPrefix(prefix);
        }

        /**
         * ��ȡ����ƥ��ģʽ��Ԫ�أ�֧�� '.' ��Ϊͨ�����ƥ�䵥�������ַ���
         * @param pattern ģʽ�ַ���
         * @return ƥ���Ԫ���б�
         */
        std::list<std::string> keysByPattern(const std::string& pattern)
        {
            return map.keysByPattern(pattern);
        }

        /**
         * �жϼ������Ƿ����ƥ��ģʽ��Ԫ�أ�֧�� '.' ��Ϊͨ�����
         * @param pattern ģʽ�ַ���
         * @return ���ڷ��� true�����򷵻� false
         */
        bool hasPattern(const std::string& pattern)
        {
            return map.hasPattern(pattern);
        }

        /**
         * ��ȡ������Ԫ�صĸ���
         * @return Ԫ������
         */
        size_t size() const
        {
            return map.size();
        }
    };

} // namespace ol

#endif // !__OL_TRIESET_H