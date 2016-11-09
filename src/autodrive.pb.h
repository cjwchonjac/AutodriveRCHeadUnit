// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: autodrive.proto

#ifndef PROTOBUF_autodrive_2eproto__INCLUDED
#define PROTOBUF_autodrive_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace com {
namespace autodrive {
namespace message {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_autodrive_2eproto();
void protobuf_AssignDesc_autodrive_2eproto();
void protobuf_ShutdownFile_autodrive_2eproto();

class LocationMessage;
class SegmentList;
class SegmentArrived;

// ===================================================================

class LocationMessage : public ::google::protobuf::Message {
 public:
  LocationMessage();
  virtual ~LocationMessage();

  LocationMessage(const LocationMessage& from);

  inline LocationMessage& operator=(const LocationMessage& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const LocationMessage& default_instance();

  void Swap(LocationMessage* other);

  // implements Message ----------------------------------------------

  LocationMessage* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const LocationMessage& from);
  void MergeFrom(const LocationMessage& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required double latitude = 1;
  inline bool has_latitude() const;
  inline void clear_latitude();
  static const int kLatitudeFieldNumber = 1;
  inline double latitude() const;
  inline void set_latitude(double value);

  // required double longitude = 2;
  inline bool has_longitude() const;
  inline void clear_longitude();
  static const int kLongitudeFieldNumber = 2;
  inline double longitude() const;
  inline void set_longitude(double value);

  // @@protoc_insertion_point(class_scope:com.autodrive.message.LocationMessage)
 private:
  inline void set_has_latitude();
  inline void clear_has_latitude();
  inline void set_has_longitude();
  inline void clear_has_longitude();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double latitude_;
  double longitude_;
  friend void  protobuf_AddDesc_autodrive_2eproto();
  friend void protobuf_AssignDesc_autodrive_2eproto();
  friend void protobuf_ShutdownFile_autodrive_2eproto();

  void InitAsDefaultInstance();
  static LocationMessage* default_instance_;
};
// -------------------------------------------------------------------

class SegmentList : public ::google::protobuf::Message {
 public:
  SegmentList();
  virtual ~SegmentList();

  SegmentList(const SegmentList& from);

  inline SegmentList& operator=(const SegmentList& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const SegmentList& default_instance();

  void Swap(SegmentList* other);

  // implements Message ----------------------------------------------

  SegmentList* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const SegmentList& from);
  void MergeFrom(const SegmentList& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .com.autodrive.message.LocationMessage locations = 1;
  inline int locations_size() const;
  inline void clear_locations();
  static const int kLocationsFieldNumber = 1;
  inline const ::com::autodrive::message::LocationMessage& locations(int index) const;
  inline ::com::autodrive::message::LocationMessage* mutable_locations(int index);
  inline ::com::autodrive::message::LocationMessage* add_locations();
  inline const ::google::protobuf::RepeatedPtrField< ::com::autodrive::message::LocationMessage >&
      locations() const;
  inline ::google::protobuf::RepeatedPtrField< ::com::autodrive::message::LocationMessage >*
      mutable_locations();

  // @@protoc_insertion_point(class_scope:com.autodrive.message.SegmentList)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::RepeatedPtrField< ::com::autodrive::message::LocationMessage > locations_;
  friend void  protobuf_AddDesc_autodrive_2eproto();
  friend void protobuf_AssignDesc_autodrive_2eproto();
  friend void protobuf_ShutdownFile_autodrive_2eproto();

  void InitAsDefaultInstance();
  static SegmentList* default_instance_;
};
// -------------------------------------------------------------------

class SegmentArrived : public ::google::protobuf::Message {
 public:
  SegmentArrived();
  virtual ~SegmentArrived();

  SegmentArrived(const SegmentArrived& from);

  inline SegmentArrived& operator=(const SegmentArrived& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const SegmentArrived& default_instance();

  void Swap(SegmentArrived* other);

  // implements Message ----------------------------------------------

  SegmentArrived* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const SegmentArrived& from);
  void MergeFrom(const SegmentArrived& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required int32 index = 1;
  inline bool has_index() const;
  inline void clear_index();
  static const int kIndexFieldNumber = 1;
  inline ::google::protobuf::int32 index() const;
  inline void set_index(::google::protobuf::int32 value);

  // required double angle = 2;
  inline bool has_angle() const;
  inline void clear_angle();
  static const int kAngleFieldNumber = 2;
  inline double angle() const;
  inline void set_angle(double value);

  // @@protoc_insertion_point(class_scope:com.autodrive.message.SegmentArrived)
 private:
  inline void set_has_index();
  inline void clear_has_index();
  inline void set_has_angle();
  inline void clear_has_angle();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double angle_;
  ::google::protobuf::int32 index_;
  friend void  protobuf_AddDesc_autodrive_2eproto();
  friend void protobuf_AssignDesc_autodrive_2eproto();
  friend void protobuf_ShutdownFile_autodrive_2eproto();

  void InitAsDefaultInstance();
  static SegmentArrived* default_instance_;
};
// ===================================================================


// ===================================================================

// LocationMessage

// required double latitude = 1;
inline bool LocationMessage::has_latitude() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void LocationMessage::set_has_latitude() {
  _has_bits_[0] |= 0x00000001u;
}
inline void LocationMessage::clear_has_latitude() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void LocationMessage::clear_latitude() {
  latitude_ = 0;
  clear_has_latitude();
}
inline double LocationMessage::latitude() const {
  // @@protoc_insertion_point(field_get:com.autodrive.message.LocationMessage.latitude)
  return latitude_;
}
inline void LocationMessage::set_latitude(double value) {
  set_has_latitude();
  latitude_ = value;
  // @@protoc_insertion_point(field_set:com.autodrive.message.LocationMessage.latitude)
}

// required double longitude = 2;
inline bool LocationMessage::has_longitude() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void LocationMessage::set_has_longitude() {
  _has_bits_[0] |= 0x00000002u;
}
inline void LocationMessage::clear_has_longitude() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void LocationMessage::clear_longitude() {
  longitude_ = 0;
  clear_has_longitude();
}
inline double LocationMessage::longitude() const {
  // @@protoc_insertion_point(field_get:com.autodrive.message.LocationMessage.longitude)
  return longitude_;
}
inline void LocationMessage::set_longitude(double value) {
  set_has_longitude();
  longitude_ = value;
  // @@protoc_insertion_point(field_set:com.autodrive.message.LocationMessage.longitude)
}

// -------------------------------------------------------------------

// SegmentList

// repeated .com.autodrive.message.LocationMessage locations = 1;
inline int SegmentList::locations_size() const {
  return locations_.size();
}
inline void SegmentList::clear_locations() {
  locations_.Clear();
}
inline const ::com::autodrive::message::LocationMessage& SegmentList::locations(int index) const {
  // @@protoc_insertion_point(field_get:com.autodrive.message.SegmentList.locations)
  return locations_.Get(index);
}
inline ::com::autodrive::message::LocationMessage* SegmentList::mutable_locations(int index) {
  // @@protoc_insertion_point(field_mutable:com.autodrive.message.SegmentList.locations)
  return locations_.Mutable(index);
}
inline ::com::autodrive::message::LocationMessage* SegmentList::add_locations() {
  // @@protoc_insertion_point(field_add:com.autodrive.message.SegmentList.locations)
  return locations_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::com::autodrive::message::LocationMessage >&
SegmentList::locations() const {
  // @@protoc_insertion_point(field_list:com.autodrive.message.SegmentList.locations)
  return locations_;
}
inline ::google::protobuf::RepeatedPtrField< ::com::autodrive::message::LocationMessage >*
SegmentList::mutable_locations() {
  // @@protoc_insertion_point(field_mutable_list:com.autodrive.message.SegmentList.locations)
  return &locations_;
}

// -------------------------------------------------------------------

// SegmentArrived

// required int32 index = 1;
inline bool SegmentArrived::has_index() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void SegmentArrived::set_has_index() {
  _has_bits_[0] |= 0x00000001u;
}
inline void SegmentArrived::clear_has_index() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void SegmentArrived::clear_index() {
  index_ = 0;
  clear_has_index();
}
inline ::google::protobuf::int32 SegmentArrived::index() const {
  // @@protoc_insertion_point(field_get:com.autodrive.message.SegmentArrived.index)
  return index_;
}
inline void SegmentArrived::set_index(::google::protobuf::int32 value) {
  set_has_index();
  index_ = value;
  // @@protoc_insertion_point(field_set:com.autodrive.message.SegmentArrived.index)
}

// required double angle = 2;
inline bool SegmentArrived::has_angle() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void SegmentArrived::set_has_angle() {
  _has_bits_[0] |= 0x00000002u;
}
inline void SegmentArrived::clear_has_angle() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void SegmentArrived::clear_angle() {
  angle_ = 0;
  clear_has_angle();
}
inline double SegmentArrived::angle() const {
  // @@protoc_insertion_point(field_get:com.autodrive.message.SegmentArrived.angle)
  return angle_;
}
inline void SegmentArrived::set_angle(double value) {
  set_has_angle();
  angle_ = value;
  // @@protoc_insertion_point(field_set:com.autodrive.message.SegmentArrived.angle)
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace message
}  // namespace autodrive
}  // namespace com

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_autodrive_2eproto__INCLUDED