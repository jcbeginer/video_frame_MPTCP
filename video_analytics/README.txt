
data = b'0' * frame_size #여기서 b'0'는 1 byte를 가진다! (not 1 bit)

UL_throughput_client_trashbit.py를 이용해서 frame size를 70 72 74 76 78 KB로 보낼 때, 
이렇게 되면 BSR index가 바뀌면서 70일 때보다 78KB일 때 throughput이 더 좋아질 것으로 예상했다!

(server to client는 constant bytes를 가지는 값을 보내도록 했고, network fluctuation은 여러번의 실험으로 커버를 하려고 했다.)
나의 예상은 당연히 78KB는 resource를 더 많이 할당받을 것이므로 70KB보다 throughput이 좋을 것으로 예상했으나, 그렇게 유의미하진 않은 것 같았다...;

이거에 대한 생각으로 든 것은 만약 두 가지의 경우 모두에 대해서 더 충분한 자원을 할당받는다면??? 그러면 당연히 78KB가 70KB보다 throughput이 높아야한다.
그런데 두 가지의 경우가 거의 엇비슷한 throughput을 보인다.

그러면 반대로 조금 더 frame_size를 줄이고 이렇게 하면 어떤 변화가 있을까???
