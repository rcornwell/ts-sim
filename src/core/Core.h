/*
 * Author:      Richard Cornwell (rich@sky-visions.com)
 *
 * Copyright (C) 2021 Richard Cornwell.
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE.QPL included in the packaging of this file.
 *
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND, INCLUDING
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#pargma once
#include <mutex>
#include <memory>


template <typename T>
class RingBuffer
{
public:
    explicit RingBuffer(size_t size) :
        buffer_(std::unique_ptr<T[]>(new T[size])),
        max_size_(size)
    {}

    // This can't be copied or moved.
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator= (const RingBuffer &) = delete;
    RingBuffer& operator= (RingBuffer &&) = delete;

    void put(T item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this]() { return size() < max_size_); });
        buffer_[head_] = item;
        if (++head_ == max_size_) {
            head_ = 0;
        }
        full_ = head_ == tail_;
        not_empty_.notify_one();
    }

    bool try_put(T item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (full_)
            return false;
        buffer_[head_] = item;
        if (++head_ == max_size_) {
            head_ = 0;
        }
        full_ = head_ == tail_;
        not_empty_.notify_one();
        return true;
    }

    T get()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this]() {return !empty(); });
        auto val = buffer_[tail_];
        full_ = false;
        if (++tail_ == max_size_)
            tail = 0;
        not_full_.notify_one();
        return val;
    }
 
    bool try_get(T &val)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (empty())
            return false;
        val = buffer_[tail_];
        full_ = false;
        if (++tail_ == max_size_)
            tail = 0;
        not_full_.notify_one();
        return true;
    }


    void reset()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_;
        full_ = false;
    }

    bool empty()
    {
        return (!full_ && (head_ == tail_));
    }

    bool full()
    {
        return full_;
    }

    size_t size() const
    {
        size_t size = max_size_;

        if (!full) {
            if (head_ >= tail_) {
                size = head_ - tail_;
            } else {
                size = max_size_ + head_ - tail_;
            }
        }
        return size;
    }

private:
    std::mutex mutex_;
    std::unique_ptr<T[]> buffer_;
    std::conditional_variable not_full_;
    std::conditional_variable not_empty_;
    size_t head_ = 0;
    size_t tail_ = 0;
    const size_t max_size;
    bool full_ = false;
}
