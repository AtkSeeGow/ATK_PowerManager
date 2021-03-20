import { Component } from '@angular/core';
import { HttpErrorHandler } from '../common/httpErrorHandler.component';
import { HomeService } from './home.service';

import { forkJoin } from 'rxjs';

declare const $: any;

@Component({
  selector: 'app-home',
  templateUrl: './home.html',
  providers: [HomeService]
})
export class HomeComponent {
  turnOnMessage: string = "";
  completeMessage: string = "";

  isRuningTurnOn: boolean = false;
  isRuningComplete: boolean = false;

  isTurnOn: boolean = false;
  isComplete: boolean = false;

  interval: number = 500;

  constructor(private homeService: HomeService) {
    var component = this;
    component.update()
    setInterval(function () {
      component.update();
    }, 5000);
  };

  update() {
    if (!this.isRuningTurnOn) {
      this.isRuningTurnOn = true;
      this.homeService.asyncReadDigitalValue().subscribe((next: any) => {
        var value = next;
        var validResult = value.validResult;
        this.turnOnMessage = "";
        if (!validResult.isValid) {
          for (let key in validResult.errorMessages)
            this.turnOnMessage = validResult.errorMessages[key];
        }
        else {
          var status = JSON.parse(validResult.successMessages.value);
          this.isTurnOn = status.value == 1;
        }
        this.isRuningTurnOn = false;
      }, (error: any) => { HttpErrorHandler.Notify(error); });
    }

    if (!this.isRuningComplete) {
      this.isRuningComplete = true;
      this.homeService.asyncStatus().subscribe((next: any) => {
        var value = next;
        var validResult = value.validResult;
        this.completeMessage = "";
        this.isComplete = validResult.isValid;
        if (!validResult.isValid) {
          for (let key in validResult.errorMessages)
            this.completeMessage = validResult.errorMessages[key];
        }
        this.isRuningComplete = false;
      }, (error: any) => { HttpErrorHandler.Notify(error); });
    }
  }

  push() {
    const notify = $.notify({ icon: "tim-icons icon-bell-55", message: "Please Wait" }, { type: 'info', delay: 0, placement: { from: 'top', align: 'right' } });
    this.homeService.asyncSwitchOpticalCoupler(this.interval).subscribe((next: any) => {
      notify.close();
      var response = next;
      var validResult = response.validResult;
      HttpErrorHandler.Notify({ error: validResult });
    }, (error: any) => { HttpErrorHandler.Notify(error); });
  }
}
