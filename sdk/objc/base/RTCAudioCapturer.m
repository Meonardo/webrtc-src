#import "RTCAudioCapturer.h"

@implementation RTC_OBJC_TYPE (RTCAudioCapturer)

@synthesize delegate = _delegate;

- (instancetype)initWithDelegate:(id<RTC_OBJC_TYPE(RTCAudioCapturerDelegate)>)delegate {
  if (self = [super init]) {
    _delegate = delegate;
  }
  return self;
}

@end
