#ifndef DUO_DATA_FILE_H_
#define DUO_DATA_FILE_H_

#include <string>
#include "typedef.h"
#include "block/DataBlock.h"


namespace DuoDuo
{
    class DataBlock;

    class DataFile
    {
    protected:
        class LastUnfilledNormalBlock
        {
        public:
            LastUnfilledNormalBlock(size_t block_size);
            ~LastUnfilledNormalBlock();

        private:
            DataBlock m_NormalBlock;
            block_index_t m_BlockIndex;
        };

        class CachedKeyValueContainer
        {
        public:
            CachedKeyValueContainer(size_t block_size) : m_BlockSize(block_size) {}
            ~CachedKeyValueContainer() {}
            void AddData(const std::string& key, const std::string& value) {m_CachedKeyValues[key] = value;}

            bool HasSmallData(void) const;
            bool HasBigData(void) const;

            // remove a BigData from cache and return it.
            //std::pair<std::string /*key*/, std::string /*value*/> PopBigData(void);

            // find a big data. pack it into bigBlock and dataOnlyBlock. remove those data from cache and return the blocks.
            // return empty vector when there is no bigData.
            std::vector<DataBlock> PopBigBlock(void);

            // find some small data that can pack into one normalBlock. remove those data from cache and return the normalBlock.
            DataBlock PopSmallBlock(void);

            // find some small data that can pack into one normalBlock and fill up it.
            ////DataBlock PopFilledSmallBlock(void);

            ////void SaveAllBigData(void); //  saving && clean those data
            ////lastnormalblock SaveAllSmallData(void);

        private:
            key_value_map_t::const_iterator FindDataCanPutIntoNormalBlock(const DataBlock& block) const;

        private:
            size_t m_BlockSize;
            key_value_map_t m_CachedKeyValues;
        };

    public:
        static DataFile* Create(const std::string& folder, const std::string& name, size_t block_size);

        DataFile(const std::string& folder, const std::string& filename, int fd, size_t block_size);

        // AddData RemoveData will only add data to cache.i
        // You should use Save or SaveFilledBlock to start storing data.
        // And you can use Flush() to ensure the data saved to disk.
        // example: saving all data && make sure data is stored to disk.
        //   datafile.AddData  datafile.AddData  datafile.RemoveData  datafile.AddData  ...
        //   datafile.Save
        //   datafile.Flush
        // or:  save all full block. and let os controll the io.
        //   datafile.AddData  ...
        //   datafile.SaveFilledBlock
        void AddData(const std::string& key, const std::string& value);
        void RemoveData(const std::string& key);

        // Save() will saving all unsaved data to file. and return the index information.
        void Save(void);

        // SaveFilledBlock() will save datas that can fill up the block. This means some data will left in m_CachedKeyValues (that can not fill up the block).
        void SaveFilledBlock(void);

        //void Flush(void);

        std::string GetFullPathFileName(void) const;

    private:
        static std::string name_to_file_name(const std::string& name);

    private:
        std::string m_Folder;
        std::string m_Filename;
        int m_Fd;
        size_t m_BlockSize;
        LastUnfilledNormalBlock m_LastUnfilledNormalBlock;
        CachedKeyValueContainer m_CachedKeyValues;
    };
}

#endif
